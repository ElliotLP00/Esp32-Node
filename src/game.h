#ifndef GAME_H_
#define GAME_H_
#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "display.h"
#include "game.h"

#define GAME "game task"
#define GAME_STACK_SIZE 2048
#define GAME_PRIORITY 1
#define GAME_PERIOD 50
#define START_PERIOD 200
void setscore(int i);
void initgame();
void calculateScore();
void IRAM_ATTR isr();
void moleMiss();
void messageAnalyzer(String s);

void initgame();
void gameloop();

#endif