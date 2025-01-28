#ifndef ENTITIES_H
#define ENTITIES_H

#include <Arduino.h>

#define maxEnemies 30

struct Damsel {
    float x, y;
    float speed;
    bool dead;
    bool followingPlayer;
};
extern Damsel damsel[1];

struct Enemy {
  float x, y;
  int hp;
  bool chasingPlayer;
  float moveAmount;
  const char* name;
  int attackDelay;
};
extern Enemy enemies[maxEnemies];

void updateEnemies(int& playerHP, float playerX, float playerY, const char*& deathCause);

#endif