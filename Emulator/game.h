#ifndef GAME_H
#define GAME_H

#include "Common.h"
#include "GameState.h"

// ─────────────────────────────────────────────────────────────────────────────
// Top-level game loop entry points
// ─────────────────────────────────────────────────────────────────────────────
void game_setup();
void game_loop();

// ─────────────────────────────────────────────────────────────────────────────
// Game flow functions (split across Screens.cpp and GameLoop.cpp)
// ─────────────────────────────────────────────────────────────────────────────
void resetGame();
void updateGame();
void handleAmbientNoiseLevel();
void renderGame();

// Screens (in Screens.cpp)
void renderIntroScreen();
void renderSecretScreen();
void renderSplashScreen();
void renderCredits();
void gameOver();
void showStatusScreen();

// Boss AI (in BossAI.cpp)
void updateBossfight();

#endif // GAME_H