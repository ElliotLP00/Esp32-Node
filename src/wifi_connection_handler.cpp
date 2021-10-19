#include "wifi_connection_handler.h"
#include <WiFi.h>
#include <WiFiMulti.h>



WiFiMulti wifiMulti; 

int clientMessage()
{
     
     int id= 0; 
    const uint16_t port = 5008;
    const char * host = "192.168.43.241"; // ip or dns

    WiFiClient client;

    wifiMulti.addAP("Elliot's telefon", "123456789Aa");

    Serial.println();
    Serial.println();
    Serial.print("Waiting for WiFi... ");

    while(wifiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    delay(500);

        if (!client.connect(host, port)) {
        Serial.println("Connection failed.");
        Serial.println("Waiting 5 seconds before retrying...");
        delay(5000);
        
    }  
    
    Serial.print("Connecting to ");
    Serial.println(host);
 
  delay(1000);
  client.println("index");
  client.flush();
  delay(500);
  
  int maxloops = 0;

  //wait for the server's reply to become available
  while (!client.available() && maxloops < 1000)
  {
    maxloops++;
    delay(1); //delay 1 msec
  }
  
  if (client.available() > 0)
  {
    //read back one line from the server
     id = client.read();
    Serial.println(id);
  }
  else
  {
    Serial.println("client.available() timed out ");
    Serial.println("Restarting");
    ESP.restart();
  }

  client.stop();

  return id; 
  
}; 