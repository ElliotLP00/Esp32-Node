#ifndef GAME_H_
#define GAME_H_
#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "display.h"
#include "game.h"

void initgame();
void calculateScore();
void IRAM_ATTR isr();
void moleMiss();
void messageAnalyzer(String s);
void initgame();
void gameloop();
#endif