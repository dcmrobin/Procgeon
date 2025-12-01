#ifndef ENTITIES_H
#define ENTITIES_H

#include <string>
#include <cstdlib>
#include "Sprites.h"
#include "Translation.h"

#define maxEnemies 30
#define maxProjectiles 30
#define maxParticles 200

struct Damsel {
  float x, y;
  float speed;
  bool dead;
  bool followingPlayer;
  bool active;
  int levelOfLove;
  char name[30];
  bool completelyRescued;
  bool beingCarried;
};
extern Damsel damsel[1];

struct PathNode {
  int x, y;
};

struct Dialogue {
  char message[200] = "";
  int duration;
  char tone[20] = "normal";
  bool alreadyBeenSaid = false;
};
extern Dialogue damselAnnoyingDialogue[10];
extern Dialogue damselPassiveDialogue[7];
extern Dialogue damselGoodDialogue[7];
extern Dialogue damselCarryDialogue[7];
extern Dialogue ridiculeDialogue[8];
extern Dialogue glamourDialogue[8];

enum BossStates {
  Idle,
  Floating,
  Shooting,
  Summoning,
  Enraged,
  Beaten
};

struct Enemy {
  float x, y;
  int hp;
  bool chasingPlayer;
  float moveAmount;
  char name[30];
  int attackDelay;
  int damage;
  bool hasWanderPath;
  int pathLength;
  int currentPathIndex;
  PathNode wanderPath[32];  // maximum length for a wandering route
  const unsigned char* sprite; // Pointer to current sprite bitmap
  int attackDelayCounter = attackDelay; // Each enemy tracks its own attack delay
  bool nearClock;
  bool isFriend;
};
extern Enemy enemies[maxEnemies];

struct Projectile {
  float x, y;
  float dx, dy;
  float speed;
  float damage;
  bool active;
  bool shotByPlayer;
  int shooterId;
};
extern Projectile projectiles[maxProjectiles];

struct Particle {
  float x, y;
  float vx, vy;  // velocity
  int lifetime;  // remaining frames
  int maxLifetime;  // for fade effect
  bool active;
};
extern Particle particles[maxParticles];

extern int levelOfDamselDeath;
extern float clockX;
extern float clockY;

extern BossStates bossState;
extern int bossStateTimer;

void updateEnemies();
void updateDamsel();
void updateProjectiles();
void moveDamselToPos(float posX, float posY);
void shootProjectile(float x, float y, float xDir, float yDir, bool shotByPlayer, int shooterId);
void renderEnemies();
void renderDamsel();
void renderProjectiles();
void reduceArmorDurability(int i);
void spawnParticles(float x, float y, int count, float speed, bool isLarge);
void updateParticles();
void renderParticles();

#endif