#ifndef PLAYER_H
#define PLAYER_H

#include <Arduino.h>

extern float playerX;
extern float playerY;
extern int playerHP;
extern int playerMaxHP;
extern bool speeding;
extern int speedTimer;
extern bool hasMap;
extern String deathCause;
extern int dungeon;
extern int kills;
extern int playerDX;
extern int playerDY;

void renderPlayer();
void handleInput();

#endif