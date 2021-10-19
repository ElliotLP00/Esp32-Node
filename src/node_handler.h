#ifndef NODE_HANDLER_H_
#define NODE_HANDLER_H_

#include <Arduino.h>
#include <esp_now.h>

#define KEEPALIVE "KeepAlive task"
#define KEEPALIVE_STACK_SIZE 2048
#define KEEPALIVE_PRIORITY 6
#define KEEPALIVE_PERIOD 1000


extern int id; 

typedef struct node_struct
{
  int id;
  uint8_t macAdress[6];
} node_struct;

int generateID();

void initArray();

void addPeer(node_struct);

void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLength);

void sendMessage(const String &message, node_struct node);

void receiveCallback(const uint8_t *macAddr, const uint8_t *data, int dataLen);

// callback when data is sent
void sentCallback(const uint8_t *macAddr, esp_now_send_status_t status);

void broadcast(const String &message);

node_struct addMacToStruct(const uint8_t *macAddr,node_struct n);

void sendMessage(const String &message, int node_id);

int getNodeIndexFromList(int node_id);
void removeNodeFromList(node_struct node);

void handleNewNodeBroadcast(node_struct n);

void handleDisconnect(node_struct node);

void handleAnswerNewNode(node_struct node);


int addToList(node_struct);

void addNeighbor(node_struct node);

void findNewNeighbor();

void printMacAddress(const uint8_t *macAddr);

String formatId();

void printList();
boolean checkIfLast();


#endif