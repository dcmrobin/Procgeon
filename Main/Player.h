#ifndef PLAYER_H
#define PLAYER_H

#include <Arduino.h>
#include "Item.h"

extern String deathCause;
extern float playerX;
extern float playerY;
extern float currentSpeedMultiplier;
extern int playerHP;
extern int playerMaxHP;
extern int speedTimer;
extern int dungeon;
extern int kills;
extern int playerDX;
extern int playerDY;
extern int ingredient1index;
extern int playerFood;
extern bool speeding;
extern bool hasMap;
extern bool paused;
extern bool carryingDamsel;
extern bool damselGotTaken;
extern bool combiningTwoItems;
extern bool playerMoving;
extern bool starving;
extern bool seeAll;
extern GameItem combiningItem1;
extern GameItem combiningItem2;

void renderPlayer();
void handleInput();
void startCarryingDamsel();
void handlePauseScreen();
void handleHunger();

#endif