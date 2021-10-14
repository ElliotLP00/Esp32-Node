#include <Arduino.h>
//Prints out time between button presses in seconds

byte button = 14;
#define led 33
unsigned long startTime;
unsigned long endTime;
unsigned long duration;
int score = 0;
byte timerRunning;
int difficulty = 1;

int connectedNodes = 0;
int activeMoles = 0;
boolean gameActive = false;

void setup() {
  pinMode (button, INPUT);
  pinMode(led, OUTPUT);
  //digitalWrite(led, HIGH); //LED starts ON
  Serial.begin(9600);
}

void loop() {

  if (digitalRead(button) == LOW) {
    delay(random(2000, 5000)); //Delay random time between 2 to 5 seconds
    digitalWrite(led, HIGH); //LED ON
  } else {
    digitalWrite(led, LOW); //LED OFF
  }


  if (timerRunning == 0 && digitalRead(button) == LOW && digitalRead(led) == HIGH) { // button not pressed & timer not running already & LED turns ON
    startTime = (millis() / 1000); // seconds
    timerRunning = 1;
    //digitalWrite(led, LOW); //LED OFF
  }

  if (timerRunning == 1 && digitalRead(button) == HIGH) { // timer running, button pressed
    endTime = (millis() / 1000); // seconds
    timerRunning = 0;
    duration = endTime - startTime;
    Serial.print ("------------------------------------\nButton press time in seconds: ");
    Serial.println (duration);
    switch (difficulty) {
      case 1: //Difficulty level 1
        if (duration <= 1,6) {
          Serial.print ("Difficulty increased 3 steps\n");
          score += 3;
        } else if (duration == 3) {
          Serial.print ("Difficulty increased 2 steps\n");
          score += 2;
        } else if (duration == 5) {
          Serial.print ("Difficulty increased 1 step\n");
          score += 1;
        } else if (duration >= 6 && score >= 1) {
          Serial.print ("Difficulty decreased 1 step\n");
          score -= 1;
        }
        break;
      case 2: //Difficulty level 2
        if (duration <= 1) {
          Serial.print ("Difficulty increased 3 steps\n");
          score += 3;
        } else if (duration == 2) {
          Serial.print ("Difficulty increased 2 steps\n");
          score += 2;
        } else if (duration == 3) {
          Serial.print ("Difficulty increased 1 step\n");
          score += 1;
        } else if (duration >= 4 && score >= 1) {
          Serial.print ("Difficulty decreased 1 step\n");
          score -= 1;
        }
        break;
      case 3: //Difficulty level 3
        if (duration <= 0,25) {
          Serial.print ("Difficulty increased 3 steps\n");
          score += 3;
        } else if (duration == 0,5) {
          Serial.print ("Difficulty increased 2 steps\n");
          score += 2;
        } else if (duration == 1) {
          Serial.print ("Difficulty increased 1 step\n");
          score += 1;
        } else if (duration >= 2 && score >= 1) {
          Serial.print ("Difficulty decreased 1 step\n");
          score -= 1;
        }
        break;
    }
    Serial.print ("Score: ");
    Serial.println (score);
    digitalWrite(led, LOW); //LED OFF
  }
  if (score <= 33) {
     difficulty == 1;
  } else if (score >= 34 && score <= 67) {
    difficulty == 2;
  } else {
    difficulty ==3;
  }
  delay(50);
}
//+05x11x7xiiiiiii-
void messageAnalyzer(String s){

  int type = s.substring(0,0).toInt();
  String data = s.substring(1,2);

  switch (type){
            case 3:
                if(activeMoles < 4 && gameActive){
                    Serial.println("Incoming Mole active");
                    activeMoles++;
                }else{
                    Serial.println("NOT POSSIBLE IN THIS STATE");
                }
                break;
            case 4:
                if(activeMoles > 0 && gameActive){
                    Serial.println("Incoming Mole hit.");
                    difficulty += data.toInt();
                    activeMoles--;
                }else{
                    Serial.println("NOT POSSIBLE IN THIS STATE");
                }
                break;
            case 5:
                if(gameActive){
                    Serial.println("Incoming Mole miss");
                    if(difficulty > 0)
                        difficulty--;
                    if(data=="Mole time exceeded" && activeMoles>0){
                        activeMoles--;
                    }else{
                        Serial.println("NOT POSSIBLE IN THIS STATE");
                    }
                }else{
                    Serial.println("NOT POSSIBLE IN THIS STATE");
                }
                break;
            case 6:
                if(!gameActive && connectedNodes >= 4){
                    Serial.println("Incoming Start game");
                    activeMoles = 0;
                    gameActive = true;
                    //new Game().start();
                }else{
                    Serial.println("NOT POSSIBLE IN THIS STATE");
                }
                break;
            case 7:
                if (gameActive){
                    Serial.println("Incoming Stop game");
                    gameActive = false;
                }else{
                    Serial.println("NOT POSSIBLE IN THIS STATE");
                }
                break;
            case 8:
                Serial.println("Incoming Node disconnected");
                connectedNodes--;
                Serial.println("Total:"+connectedNodes);
                break;
        }
}