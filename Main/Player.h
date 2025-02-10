#ifndef PLAYER_H
#define PLAYER_H

#include <Arduino.h>

extern String deathCause;
extern float playerX;
extern float playerY;
extern int playerHP;
extern int playerMaxHP;
extern int speedTimer;
extern int dungeon;
extern int kills;
extern int playerDX;
extern int playerDY;
extern bool speeding;
extern bool hasMap;
extern bool paused;

void renderPlayer();
void handleInput();

#endif