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

struct Dialogue {
  String message = "";
  int duration;
  String tone = "normal";
  bool alreadyBeenSaid = false;
};

extern Dialogue damselAnnoyingDialogue[] = {
  {"Why are you even down here, anyway?", 500},
  {"You had better not die.", 400},
  {"I hate this place!", 300, "annoying"},
  {"Why do wizards wear that stupid hat?", 500, "annoying"},
  {"Hurry up, will you?", 300},
  {"You expect a thanks, don't you? Not gonna happen.", 500, "annoying"},
  {"Does this place ever end?", 350},
  {"For the record, I don't really like you.", 500, "annoying"},
  {"When was the last time you had a shower?", 500, "annoying"},
  {"I could probably get out on my own.", 400}
};

extern Dialogue damselPassiveDialogue[] = {
  {"Huh, maybe you're not as dumb as I thought.", 500},
  {"Maybe you know where you're going after all...", 500},
  {"I apologize if I said anything hurtful.", 500, "alone"},
  {"Please don't die.", 300, "alone"},
  {"I probably couldn't get out on my own.", 500},
  {"Thanks for rescuing me, anyway.", 400, "alone"},
  {"Well, at least I'm not alone anymore.", 500}
};

extern Dialogue damselGoodDialogue[] = {
  {"Actually, I do kind of like you.", 400, "alone"},
  {"I'm sorry if I was annoying you.", 400},
  {"Do you mind... carrying me? (Press [B] next to me)", 400, "alone"},
  {"Does that staff ever run out?", 350},
  {"I'm glad I'm with you.", 300},
  {"I don't want to die...", 300, "annoying"},
  {"Do you think we're almost at the end?", 500}
};

extern Dialogue damselCarryDialogue[] = {
  {"You're actually kind of strong...", 450},
  {"Can I stay in your arms for a bit?", 450},
  {"You can put me down if you want. (Hold [B])", 500},
  {"I kind of like it here...", 300},
  {"Can we... never mind.", 300},
  {"I don't want to be in that cell ever again!", 500},
  {"Maybe I can stay with you after we escape?", 500}
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

void updateEnemies();
void updateDamsel();
void updateProjectiles();
void moveDamselToPos(float posX, float posY);
void shootProjectile(float xDir, float yDir);
void renderEnemies();
void renderDamsel();
void renderProjectiles();

#endif
