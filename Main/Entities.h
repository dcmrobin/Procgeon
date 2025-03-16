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
  String name;
};
extern Damsel damsel[1];

struct PathNode {
  int x, y;
};

struct Enemy {
  float x, y;
  int hp;
  bool chasingPlayer;
  float moveAmount;
  String name;
  int attackDelay;
  int damage;
  bool hasWanderPath;
  int pathLength;
  int currentPathIndex;
  PathNode wanderPath[32];  // maximum length for a wandering route
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

extern int levelOfDamselDeath;

void trainFemaleMarkov();
String generateFemaleName();
void updateEnemies();
void updateDamsel();
void updateProjectiles();
void moveDamselToPos(float posX, float posY);
void shootProjectile(float xDir, float yDir);
void renderEnemies();
void renderDamsel();
void renderProjectiles();

#endif
