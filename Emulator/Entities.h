#ifndef ENTITIES_H
#define ENTITIES_H

#include <string>
#include <cstdlib>

#define maxEnemies 30
#define maxProjectiles 30

struct Damsel {
  float x, y;
  float speed;
  bool dead;
  bool followingPlayer;
  bool active;
  int levelOfLove;
  std::string name;
};
extern Damsel damsel[1];

struct PathNode {
  int x, y;
};

struct Dialogue {
  std::string message = "";
  int duration;
  std::string tone = "normal";
  bool alreadyBeenSaid = false;
};
extern Dialogue damselAnnoyingDialogue[10];
extern Dialogue damselPassiveDialogue[7];
extern Dialogue damselGoodDialogue[7];
extern Dialogue damselCarryDialogue[7];
extern Dialogue ridiculeDialogue[8];
extern Dialogue glamourDialogue[8];

struct Enemy {
  float x, y;
  int hp;
  bool chasingPlayer;
  float moveAmount;
  std::string name;
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

void updateEnemies();
void updateDamsel();
void updateProjectiles();
void moveDamselToPos(float posX, float posY);
void shootProjectile(float xDir, float yDir);
void renderEnemies();
void renderDamsel();
void renderProjectiles();

#endif