#include <esp_now.h>
#include <esp_task_wdt.h>
#include "esp_err.h"
#include <Arduino.h>
#include "node_handler.h"
#include "game.h"

// HÅLLER PÅ ATT BYTA ID
int indext = 0;
bool lastNode = true;
int id = 0; // 1, 2, eller 3
int connected_nodes = 1;
int myIndex = 0;
// node_struct node;
node_struct neighbor;
boolean hasSentKeepAlive = false;
uint64_t timeLastMessage;
node_struct nodes[16];
boolean boolConfirmedNode;

static void keepAliveCheckTask(void *arg)
{
  while (1)
  {
    if (neighbor.id > 0)
    {

      uint64_t now = esp_timer_get_time();
      uint64_t timeDiff = now - timeLastMessage;
      timeDiff /= 1000;

      if (timeDiff > 5000 && timeDiff < 10000 && hasSentKeepAlive == false)
      {
        ////Serial.println();
        ////Serial.println("Time since last neighbor message is > 5s");
        // Skicka keepalive-meddelande
        String str = "c";
        str += formatId();
        sendMessage(str, neighbor.id);
        ////Serial.print("in keep alive taskn enighbor id is ");
        ////Serial.println(neighbor.id);
        hasSentKeepAlive = true; // Ska sättas till false igen antingen när svar från nod inkommer eller när den tas bort som granne.
      }
      else if (timeDiff >= 10000 && boolConfirmedNode == true)
      {
        ////Serial.println();
        ////Serial.println("Time since last neighbor message is > 10s");
        boolConfirmedNode = false;
        String str = "c";
        str += formatId();
        int oldNeighborId = neighbor.id;
        handleDisconnect(neighbor);
        findNewNeighbor();
        sendMessage(str, neighbor);
        str = "f";
        str += formatId();
        str += oldNeighborId;

        broadcast(str);
      }
      else if (timeDiff > 1000 && boolConfirmedNode == false)
      {
        handleDisconnect(neighbor);
        findNewNeighbor();
        String str = "c";
        str += id;
        sendMessage(str, neighbor);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(KEEPALIVE_PERIOD));
  }
}

static void generateMoleTask(void *arg)
{
  while (1)
  {
    int temp = random(100);
    if (temp > 90)
    {
      String str = "e";
      str += id;
      vTaskDelay(pdMS_TO_TICKS(2000));
      broadcast(str);
    }
  }
}

void initArray()
{
  for (int i = 0; i < 16; i++)
  {
    nodes[i].id = 0;
  }
  neighbor.id = 0;
  xTaskCreate(keepAliveCheckTask, KEEPALIVE, KEEPALIVE_STACK_SIZE, NULL, KEEPALIVE_PRIORITY, NULL);
  //   xTaskCreate(generateMoleTask, "generate mole", KEEPALIVE_STACK_SIZE, NULL, KEEPALIVE_PRIORITY, NULL);
  ////Serial.print("My id is ");
  ////Serial.println(id);
  
  setnodeID(id);
  displayStartText();
  initgame();
}

void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength)
{
  snprintf(buffer, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

void addPeer(node_struct node)
{

  esp_now_peer_info_t peerInfo = {};
  memcpy(&peerInfo.peer_addr, node.macAdress, 6);
  esp_err_t temp = esp_now_add_peer(&peerInfo);
  if (temp != ESP_OK)
  {

    ////Serial.println(esp_err_to_name(temp));
    ////Serial.println("Failed to add peer");
    return;
  }
}

void sendMessage(const String &message, node_struct node)
{

  //  This will send a message to a specific device
  //    //Serial.print("In send message. receiver id is ");
  //    //Serial.println(node.id);

  esp_err_t result = esp_now_send(node.macAdress, (const uint8_t *)message.c_str(), message.length());

  if (result == ESP_OK)
  {
    ////Serial.println("Personal message success");
  }
  else if (result == ESP_ERR_ESPNOW_NOT_INIT)
  {
    ////Serial.println("ESPNOW not Init.");
  }
  else if (result == ESP_ERR_ESPNOW_ARG)
  {
    ////Serial.println("Invalid Argument");
  }
  else if (result == ESP_ERR_ESPNOW_INTERNAL)
  {
    ////Serial.println("Internal Error");
  }
  else if (result == ESP_ERR_ESPNOW_NO_MEM)
  {
    ////Serial.println("ESP_ERR_ESPNOW_NO_MEM");
  }
  else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
  {
    ////Serial.println("Peer not found.");
  }
  else
  {
    ////Serial.println("Unknown error");
  }
}

void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen)
{
  node_struct node;

  uint64_t now = esp_timer_get_time();
  // only allow a maximum of 250 characters in the message + a null terminating byte
  char buffer[ESP_NOW_MAX_DATA_LEN + 1];
  int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
  strncpy(buffer, (const char *)data, msgLen);
  // make sure we are null terminated
  buffer[msgLen] = 0;
  // format the mac address
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
  // debug log the message to the serial port
  //Serial.printf("Received message from: %s - %s\n", macStr, buffer);
  node = addMacToStruct(macAddr, node);
  String temp = buffer;
  String msgType = temp.substring(0, 1);
  String senderId = temp.substring(1, 3);
  String dataContent = temp.substring(3, 5);

  node.id = senderId.toInt();
  if (node.id == 0)
  {
    ////Serial.print("node id is 0 in receiveCallback, message was ");
    //Serial.println(temp);
  }

  vTaskDelay(pdMS_TO_TICKS(100));

  if (msgType == "a") // New node broadcast: alla noder lägger till den nya noden till sin array.
  {

    if (!addToList(node))
    {
      ////Serial.println("failed to add node to node list in messagetype a");
    }
    connected_nodes++;
    if (checkIfLast() == true) // Den som är nästsist får den ny tillagda nodes som granne.
    {
      addNeighbor(node);
    }
    lastNode = false;
    String str = "b";
    str += formatId();
    addPeer(node);
    sendMessage(str, node);
  }

  else if (msgType == "b") // Svar på new node broadcast: mottas av den nya noden, skickas av alla andra noder.
  {
    connected_nodes++;

    if (!addToList(node))
    {
      ////Serial.println("failed to add node to node list in messagetype b");
    }
    uint8_t tempId = 0;
    uint8_t lowId = 255;

    for (int i = 0; i < 16; i++)
    {
      tempId = nodes[i].id;
      if (tempId < lowId && tempId != 0)
      {
        lowId = tempId;
      }
    }

    int index = getNodeIndexFromList(lowId);
    addNeighbor(nodes[index]);
    addPeer(node);
  }
  else if (msgType == "c") // Keep alive mottaget
  {
    ////Serial.println("keep alive message received");
    String reply = "d";
    reply += formatId();
    sendMessage(reply, node.id);
  }
  else if (msgType == "d") // Ack
  {
    ////Serial.println("ack received");
  }
  else if (msgType == "e") // Mole
  {
    ////Serial.print("received mole message from ");
    //Serial.println(node.id);
    messageAnalyzer(dataContent);
  }
  else if (msgType == "f")
  {
    int i = dataContent.toInt();
    int j = getNodeIndexFromList(i);
    node_struct n = nodes[j];
    handleDisconnect(n);
    ////Serial.printf("received dc node message from %d, dc node is %d\n", node.id, i);
  }
  else
  {
    ////Serial.println("Message type is not recognised as valid");
  }

  if (neighbor.id == node.id) // Om sändaren är ens granne, spara tiden för meddelandet.
  {
    timeLastMessage = now;
    hasSentKeepAlive = false;
    boolConfirmedNode = true;
  }
  ////Serial.printf("connected nodes %d\n", connected_nodes);
  printList();
}

void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status)
{
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
//  //Serial.print("Last Packet Sent to: ");
//  //Serial.println(macStr);
 // //Serial.print("Last Packet Send Status: ");
 // //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void printMacAddress(const uint8_t *macAddr)
{
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
  ////Serial.print("MAC address: ");
  ////Serial.println(macStr);
}

void broadcast(const String &message)
{
  // this will broadcast a message to everyone in range
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_now_peer_info_t peerInfo = {};
  memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
  if (!esp_now_is_peer_exist(broadcastAddress))
  {
    esp_now_add_peer(&peerInfo);
  }

  esp_err_t result = esp_now_send(broadcastAddress, (const uint8_t *)message.c_str(), message.length());

  // and this will send a message to a specific device
  /*
  uint8_t peerAddress[] = {0x3C, 0x71, 0xBF, 0x47, 0xA5, 0xC0};
  esp_now_peer_info_t peerInfo = {};
  memcpy(&peerInfo.peer_addr, peerAddress, 6);
  if (!esp_now_is_peer_exist(peerAddress))
  {
    esp_now_add_peer(&peerInfo);
  }
  esp_err_t result = esp_now_send(peerAddress, (const uint8_t *)message.c_str(), message.length());*/
  if (result == ESP_OK)
  {
    //   //Serial.println("Broadcast message success");
  }
  else if (result == ESP_ERR_ESPNOW_NOT_INIT)
  {
         //Serial.println("ESPNOW not Init.");
  }
  else if (result == ESP_ERR_ESPNOW_ARG)
  {
         //Serial.println("Invalid Argument");
  }
  else if (result == ESP_ERR_ESPNOW_INTERNAL)
  {
         //Serial.println("Internal Error");
  }
  else if (result == ESP_ERR_ESPNOW_NO_MEM)
  {
         //Serial.println("ESP_ERR_ESPNOW_NO_MEM");
  }
  else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
  {
          //Serial.println("Peer not found.");
  }
  else
  {
    //Serial.println("Unknown error");
  }
}

node_struct addMacToStruct(const uint8_t *macAddr, node_struct n)
{

  for (int i = 0; i < 6; i++)
  {
    n.macAdress[i] = macAddr[i];
  }

  return n;
}
// Tar emot en node_struct med en viss nodeid och macadress.
void addNeighbor(node_struct node)
{
  for (int j = 0; j < 6; j++)
  {
    neighbor.macAdress[j] = node.macAdress[j];
  }

  neighbor.id = node.id;
  //Serial.print("My neighbor is");
  ////Serial.println(neighbor.id);
}

void findNewNeighbor()
{
  if (connected_nodes == 1)
  {
    neighbor.id = 0;
  }
  else if (connected_nodes > 1)
  {

    ////Serial.println("finding new neighbor");
    uint8_t newNeighborId = 255;
    uint8_t lowID = 255;

    for (node_struct node : nodes)
    {
      if (node.id < lowID && node.id != 0)
      {
        lowID = node.id;
      }
      if (node.id > id && node.id < newNeighborId && node.id > 0)
      {
        newNeighborId = node.id;
      }
    }
    if (newNeighborId == 255)
    { // Det fanns inget id som var högre än mitt eget. Sätt neighbor id till det lägsta.
      int index = getNodeIndexFromList(lowID);
      addNeighbor(nodes[index]);
    }
    else
    {
      int index = getNodeIndexFromList(newNeighborId);
      addNeighbor(nodes[index]);
    }
  }
  else {
    //Serial.printf("Connected nodes are 0 in finding new neighbor");
  }
}
void handleDisconnect(node_struct node)
{
  if (connected_nodes > 2)
  {
    removeNodeFromList(node);
    if (esp_now_del_peer(node.macAdress) == ESP_OK)
    {
      ////Serial.println("Peer has been successfully removed from peer list");
    }
  }
  connected_nodes--;
}

void sendMessage(const String &message, int node_id)
{
  int index = getNodeIndexFromList(node_id);
  node_struct node = nodes[index];
  uint8_t *addr = node.macAdress;
  esp_err_t result = esp_now_send(addr, (const uint8_t *)message.c_str(), message.length());
}

int getNodeIndexFromList(int node_id)
{
  int temp = 0;
  for (int i = 0; i < 16; i++)
  {
    node_struct node = nodes[i];
    if (node.id == node_id)
    {
      temp = i;
      break;
    }
  }
  return temp;
}
void removeNodeFromList(node_struct node)
{
  for (int i = 0; i < 16; i++)
  {
    if (nodes[i].id == node.id)
    {
      nodes[i].id = 0;
      break;
    }
  }
}
String formatId()
{
  String temp;
  if (id < 10)
  {
    temp = "0";
    temp += id;
  }
  else
    temp = id;
  return temp;
}

int addToList(node_struct node)
{
  for (int i = 0; i < 16; i++)
  {

    if (nodes[i].id == 0)
    {
      nodes[i] = node;
      return 1;
    }
  }
  return 0;
}
void printList()
{
  /*
  ////Serial.println("Printing nodes");
  for (node_struct node : nodes)
  {
    ////Serial.println(node.id);
  }*/
}
boolean checkIfLast(){

    uint8_t highestID = 0;
    for (node_struct node : nodes)
    {
      if (node.id > highestID)
      {
        highestID = node.id;
      }
      
    }
    if (id > highestID)
    { 
      return true;
    }
    else 
      return false;
    
  }
  

