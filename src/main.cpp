#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_now.h>
#include <esp_task_wdt.h>
#include "esp_err.h"
#include "wifi_connection_handler.h"
#include "node_handler.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <string.h>
#include "display.h"
#include "game.h"

void setup() {
  initdisplay();
  displayStartText();
  Serial.begin(9600);
  vTaskDelay(pdMS_TO_TICKS(10000));

  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);
  Serial.println("ESPNow Example");
  // Output my MAC address - useful for later
  Serial.print("My MAC Address is: ");
  Serial.println(WiFi.macAddress());
  id = clientMessage();
  // shut down wifi
  WiFi.disconnect();
  // startup ESP Now
  if (esp_now_init() == ESP_OK)
  {
    Serial.println("ESPNow Init Success");
    esp_now_register_recv_cb(receiveCallback);
    esp_now_register_send_cb(sentCallback);
  }
  else
  {
    Serial.println("ESPNow Init Failed");
    delay(3000);
    ESP.restart();
  }
  initArray();
  String str = "a";
  str += formatId();
  broadcast(str);
  
}

void loop(){
  vTaskDelay(pdMS_TO_TICKS(500));
}
