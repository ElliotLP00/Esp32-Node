#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);
#define I2C_SDA 33
#define I2C_SCL 32

String dnode = "-";
String dscore = "-";
String dlevel = "-";
String dtime = "-";

void initdisplay(){
    Wire.begin(I2C_SDA, I2C_SCL);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
}

void wackprintupdate(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.print("Node: ");
  display.print(dnode);
  
  display.setCursor(0, 13);
  display.print("Score: ");
  display.print(dscore);

  display.setCursor(0, 26);
  display.print("Level: ");
  display.print(dlevel);

  display.setCursor(0, 39);
  display.print("Time: ");
  display.print(dtime);
  
  display.display(); 
}

void wackprintnodeID(int ID){
  dnode = String(ID);
  wackprintupdate();
}

void wackprintscore(int score){
  dscore = String(score);
  wackprintupdate();
}

void wackprintlevel(int level){
  dlevel = String(level);
  wackprintupdate();
}

void wackprinttime(int time){
  dtime = String(time);
  wackprintupdate();
}