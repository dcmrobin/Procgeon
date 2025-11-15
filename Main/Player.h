#ifndef PLAYER_H
#define PLAYER_H

#include <Arduino.h>
#include "Item.h"
#include "Puzzles.h"

#define RIDICULE_DURATION 1000

extern String deathCause;
extern String currentDialogue;
extern float playerX;
extern float playerY;
extern float currentSpeedMultiplier;
extern int playerHP;
extern int playerMaxHP;
extern int speedTimer;
extern int seeAllTimer;
extern int dungeon;
extern int kills;
extern int playerDX;
extern int playerDY;
extern int ingredient1index;
extern int playerFood;
extern int dialogueTimeLength;
extern int timeTillNextDialogue;
extern bool speeding;
extern bool hasMap;
extern bool paused;
extern bool carryingDamsel;
extern bool damselGotTaken;
extern bool damselSayThanksForRescue;
extern bool knowsDamselName;
extern bool combiningTwoItems;
extern bool playerMoving;
extern bool starving;
extern bool seeAll;
extern bool showDialogue;
extern GameItem combiningItem1;
extern GameItem combiningItem2;
extern bool playerActed;
extern bool confused;
extern int confusionTimer;
extern bool succubusIsFriend;
extern bool nearSuccubus;
extern bool damselWasFollowing;
extern int damselWaitUpTimer;
extern bool damselSaidWaitUp;
extern int equippedArmorValue;
extern GameItem equippedArmor;
extern bool equippedRiddleStone;
extern int playerAttackDamage;
extern int swiftnessRingsNumber;
extern bool ringOfStrengthActive;
extern bool ringOfWeaknessActive;
extern bool ringOfHungerActive;
extern bool ringOfRegenActive;
extern float lastPotionSpeedModifier;
extern bool ridiculed;
extern int ridiculeTimer;
extern bool glamoured;
extern int glamourTimer;
extern bool blinded;
extern int blindnessTimer;
extern bool paralyzed;
extern int paralysisTimer;
extern bool playerNearClockEnemy;

void renderPlayer();
void handleInput();
void startCarryingDamsel();
void handlePauseScreen();
void handleHungerAndEffects();
void handleDialogue();
void handleRiddles();
void playDamselSFX(String tone);
void handleRingEffects();
void OpenChest(int cy, int cx, int dx);

#endif