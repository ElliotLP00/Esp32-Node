#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "display.h"
#include "game.h"
void setup() {
  Serial.begin(9600);
  initdisplay();
  initgame();
}

void loop() {
  gameloop();
}