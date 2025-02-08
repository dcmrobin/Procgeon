#ifndef ENTITIES_H
#define ENTITIES_H

#include <Arduino.h>

#define maxEnemies 30
#define maxProjectiles 30

struct Damsel {
    float x, y;
    float speed;
    bool dead;
    bool followingPlayer;
    bool active;
    int levelOfLove;
};
extern Damsel damsel[1];

struct Enemy {
  float x, y;
  int hp;
  bool chasingPlayer;
  float moveAmount;
  String name;
  int attackDelay;
};
extern Enemy enemies[maxEnemies];

struct Projectile {
  float x, y;
  float dx, dy;
  float speed;
  float damage;
  bool active;
};
extern Projectile projectiles[maxProjectiles];

void updateEnemies(int& playerHP, float playerX, float playerY, String& deathCause);
void updateDamsel(int playerDX, int playerDY, float playerX, float playerY);
void updateProjectiles(int& kills, int& levelOfDamselDeath, int level);
void moveDamselToPos(float posX, float posY);
void shootProjectile(float xDir, float yDir, float playerX, float playerY);
void renderEnemies(float playerX, float playerY);
void renderDamsel(float playerX, float playerY);
void renderProjectiles();

#endif