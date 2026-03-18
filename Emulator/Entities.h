#ifndef ENTITIES_H
#define ENTITIES_H

#include "Common.h"
#include "GameState.h"  // for PathNode, Dialogue, BossStates
#include "Sprites.h"    // for sprite pointers

// ─────────────────────────────────────────────────────────────────────────────
// Structs
// ─────────────────────────────────────────────────────────────────────────────

struct Damsel {
    float   x = 0.0f, y = 0.0f;
    float   speed = 0.1f;
    bool    dead             = false;
    bool    followingPlayer  = false;
    bool    active           = false;
    int     levelOfLove      = 0;
    char    name[30]         = "";
    bool    completelyRescued= false;
    bool    beingCarried     = false;
};

struct Enemy {
    float   x = 0.0f, y = 0.0f;
    int     hp              = 0;
    bool    chasingPlayer   = false;
    float   moveAmount      = 0.05f;
    char    name[30]        = "";
    int     attackDelay     = 20;
    int     damage          = 0;
    bool    hasWanderPath   = false;
    int     pathLength      = 0;
    int     currentPathIndex= 0;
    PathNode wanderPath[ASTAR_MAX_NODES] = {};
    const unsigned char* sprite = nullptr;
    int     attackDelayCounter  = 20;
    bool    nearClock       = false;
    bool    isFriend        = false;
};

struct Projectile {
    float   x = 0.0f, y = 0.0f;
    float   dx = 0.0f, dy = 0.0f;
    float   speed   = 0.0f;
    float   damage  = 0.0f;
    bool    active      = false;
    bool    shotByPlayer= false;
    int     shooterId   = -1;
};

struct Particle {
    float   x = 0.0f, y = 0.0f;
    float   vx = 0.0f, vy = 0.0f;
    int     lifetime    = 0;
    int     maxLifetime = 0;
    bool    active      = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// Global entity arrays (defined in Entities.cpp)
// ─────────────────────────────────────────────────────────────────────────────
extern Damsel       damsel[1];
extern Enemy        enemies[MAX_ENEMIES];
extern Projectile   projectiles[MAX_PROJECTILES];
extern Particle     particles[MAX_PARTICLES];

extern float        clockX;
extern float        clockY;

// ─────────────────────────────────────────────────────────────────────────────
// Dialogue tables (defined in Entities.cpp)
// ─────────────────────────────────────────────────────────────────────────────
extern Dialogue damselAnnoyingDialogue[10];
extern Dialogue damselPassiveDialogue[7];
extern Dialogue damselGoodDialogue[7];
extern Dialogue damselCarryDialogue[7];
extern Dialogue ridiculeDialogue[8];
extern Dialogue glamourDialogue[8];

// ─────────────────────────────────────────────────────────────────────────────
// Entity update / render functions (defined in Entities.cpp)
// ─────────────────────────────────────────────────────────────────────────────
void updateEnemies();
void updateDamsel();
void updateProjectiles();
void moveDamselToPos(float posX, float posY);
void shootProjectile(float x, float y, float xDir, float yDir, bool shotByPlayer, int shooterId);
void renderEnemies();
void renderDamsel();
void renderProjectiles();
void reduceArmorDurability(int enemyIndex);
void spawnParticles(float x, float y, int count, float speed, bool isLarge);
void updateParticles();
void renderParticles();

#endif // ENTITIES_H