#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "display.h"

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};
Button button1 = {26, 0, false};
#define led 14
int difficultytime [3] = {5000, 3000, 1000};
double pushTime = 200;
double lastpush = -pushTime;
double now;

double startTime;
double endTime;
double duration;
double timer;
double endTimer;
int score = 0;
byte timerRunning;
int difficulty = 1;

int activeMoles = 0;
int moleActive = false;
int gameActive = false;
double cooldownTime = 0;
int buttonPressed = false;
double buttonPressTime = 0;

//Calculate the score depending on the current difficulty level and time between LED ON and button press
void calculateScore() {
  endTime = ((double) millis() / 1000); // seconds
  timerRunning = 0;
  duration = endTime - startTime;
  Serial.println ("------------------------------------");
  Serial.print ("Button press time in seconds: ");
  Serial.println (duration);
  switch (difficulty) {
    case 1: //Difficulty level 1
      if (duration <= 1.6) {
        Serial.println ("Difficulty increased 3 steps");
        score += 3;
      } else if (duration > 1.6 && duration <= 3) {
        Serial.println ("Difficulty increased 2 steps");
        score += 2;
      } else if (duration > 3 && duration <= 5) {
        Serial.println ("Difficulty increased 1 step");
        score += 1;
      } else if (duration > 5 && score >= 1) {
        Serial.println ("Difficulty decreased 1 step");
        score -= 1;
      }
      break;
    case 2: //Difficulty level 2
      if (duration <= 1) {
        Serial.println ("Difficulty increased 3 steps");
        score += 3;
      } else if (duration == 2) {
        Serial.println ("Difficulty increased 2 steps");
        score += 2;
      } else if (duration == 3) {
        Serial.println ("Difficulty increased 1 step");
        score += 1;
      } else if (duration > 3 && score >= 1) {
        Serial.println ("Difficulty decreased 1 step");
        score -= 1;
      }
      break;
    case 3: //Difficulty level 3
      if (duration <= 0.25) {
        Serial.println ("Difficulty increased 3 steps");
        score += 3;
      } else if (duration == 0.5) {
        Serial.println ("Difficulty increased 2 steps");
        score += 2;
      } else if (duration == 1) {
        Serial.println ("Difficulty increased 1 step");
        score += 1;
      } else if (duration > 1 && score >= 1) {
        Serial.println ("Difficulty decreased 1 step");
        score -= 1;
      }
      break;
  }
  Serial.print ("Score: ");
  Serial.println (score);
  digitalWrite(led, LOW); //LED OFF
  button1.pressed = false;
  //vTaskDelay(pdMS_TO_TICKS(random(2000, 5000)));
}
//Button press
void IRAM_ATTR isr() {
  if (gameActive) {
    now = millis();
    if ((now - lastpush) > pushTime) {//Check that the button press is real (if at least 0,2 seconds has gone by since last press)
      if (timerRunning == 1) { // If timer is running and button is pressed then LED turns OFF and calculateScore() is called
        digitalWrite(led, LOW);
        calculateScore();
        button1.pressed = true;
        cooldownTime = (double) millis() + (double)random(3000, 9000);
      } else {
        Serial.println("------------------------------------");
        Serial.println("Button missed");
        if (score >= 1) {
          score -= 1;
          Serial.println("Difficulty decreased 1 step");
        }
        Serial.print("Score: ");
        Serial.println(score);
      }
      lastpush = now;
    }
  }
}

void moleMiss() {
  if (millis() - timer >= difficultytime[difficulty - 1]) { //Change difficulty 'life span' depending on difficulty level
    digitalWrite(led, LOW);
    Serial.println("------------------------------------");
    Serial.println("Button missed");
    if (score >= 1) {
      score -= 1;
      Serial.println("Difficulty decreased 1 step");
    }
    Serial.print("Score: ");
    Serial.println(score);
    timer = 0;
    timerRunning = 0;
    cooldownTime = (double) millis() + (double)random(3000, 9000);
    
  }
}

void messageAnalyzer(String s) {
  int type = s.substring(0, 0).toInt();
  String data = s.substring(1, 2);

  switch (type) {
    case 3:
      if (activeMoles < 4 && gameActive) {
        Serial.println("Incoming Mole active");
        activeMoles++;
      } else {
        Serial.println("NOT POSSIBLE IN THIS STATE");
      }
      break;
    case 4:
      if (activeMoles > 0 && gameActive) {
        Serial.println("Incoming Mole hit.");
        difficulty += data.toInt();
        activeMoles--;
      } else {
        Serial.println("NOT POSSIBLE IN THIS STATE");
      }
      break;
    case 5:
      if (gameActive) {
        Serial.println("Incoming Mole miss");
        if (difficulty > 0)
          difficulty--;
        if (data == "Mole time exceeded" && activeMoles > 0) {
          activeMoles--;
        } else {
          Serial.println("NOT POSSIBLE IN THIS STATE");
        }
      } else {
        Serial.println("NOT POSSIBLE IN THIS STATE");
      }
      break;
    case 6:
      if (!gameActive/* && connectedNodes >= 4*/) {
        Serial.println("Incoming Start game");
        activeMoles = 0;
        gameActive = true;
      } else {
        Serial.println("NOT POSSIBLE IN THIS STATE");
      }
      break;
    case 7:
      if (gameActive) {
        Serial.println("Incoming Stop game");
        gameActive = false;
      } else {
        Serial.println("NOT POSSIBLE IN THIS STATE");
      }
      break;
  }
}

void initgame(){
    pinMode (button1.PIN, INPUT_PULLUP);
    pinMode(led, OUTPUT);
    attachInterrupt(button1.PIN, isr, FALLING);
    displayStartText();
}

void gameloop(){
  int r = digitalRead(button1.PIN);
  if (r == LOW && !buttonPressed) {
    buttonPressed = true;
    buttonPressTime = millis();
    Serial.println("Button pressed");
  }else if (r == HIGH){
    buttonPressTime = 0;
    buttonPressed = false;
  }
  if ((buttonPressTime + 3000) == millis()) {
    Serial.println("Button held for 3 sec");
    //code for sending out "gameStart" to all
    for(int i = 6; i>0; i--){
      digitalWrite(led, HIGH);
      delay(i*100);
      digitalWrite(led, LOW);
      delay(i*100);
    }
    cooldownTime = (double) millis() + (double)random(3000, 9000);
    gameActive = true;
    Serial.println("GAME STARTED\n");
  }
  delay(20);  
  //Change the 'life span' of the LED depending on the difficulty level
  //Level 1: 5 seconds
  //Level 2: 3 seconds
  //Level 3: 1 seconds
  while(gameActive){
    wackprintscore(score);
    wackprintlevel(difficulty);
    if (timerRunning == 0 && button1.pressed == false && cooldownTime == 0) {
    digitalWrite(led, HIGH); //LED ON
    startTime = ((double) millis() / 1000); // seconds
    timer = millis();
    timerRunning = 1;
    } else if (cooldownTime < millis() && timerRunning == 0) {
    cooldownTime = 0;
    button1.pressed = false;

    } else if (timerRunning == 1) {
    moleMiss();
    }

    if (score <= 33) {
    difficulty = 1;
    // Code for 25% active nodes
    } else if (score >= 34 && score <= 67) {
    difficulty = 2;
    // Code for 37,5% active nodes
    } else {
    difficulty = 3;
    // Code for 50% active nodes
    }
    }
}