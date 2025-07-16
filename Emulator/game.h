#ifndef GAME_H
#define GAME_H

#include <stdint.h>

// Game state functions
void game_setup();
void game_loop();

// Game state variables (extern declarations)
extern bool itemResultScreenActive;
extern unsigned long lastUpdateTime;
extern const unsigned long frameDelay;
extern int page;
extern bool leftDamsel;

// Game functions
void resetGame();
void updateGame();
void renderGame();
void gameOver();
void showStatusScreen();

#endif