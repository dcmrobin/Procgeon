#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include "Common.h"
#include "GameState.h"
#include "Dungeon.h"
#include "Entities.h"
#include "Sprites.h"

// ─────────────────────────────────────────────────────────────────────────────
// HelperFunctions.h
//
// Utility functions only — collision detection, visibility, screen shake,
// button handling, UI rendering, name generation, save/load wrappers.
//
// Does NOT contain game logic (that lives in Player, Entities, Dungeon, etc.)
// ─────────────────────────────────────────────────────────────────────────────

// ── Riddle data (defined in HelperFunctions.cpp) ──────────────────────────
extern RiddleAnswer possibleAnswers[];
extern const int    numAnswers;

// ── Konami code ───────────────────────────────────────────────────────────
extern const KonamiInput konamiCode[];
extern const int         konamiLength;

// ── Viewport (compile-time constants exposed as runtime values) ───────────
extern const int   viewportWidth;
extern const int   viewportHeight;
extern const float scrollSpeed;

// ─────────────────────────────────────────────────────────────────────────────
// Input
// ─────────────────────────────────────────────────────────────────────────────
void updateButtonStates();
void handleUIStateTransitions();

// ─────────────────────────────────────────────────────────────────────────────
// Collision / geometry
// ─────────────────────────────────────────────────────────────────────────────
int  predictXtile(float x);
int  predictYtile(float y);
bool checkSpriteCollisionWithTileX(float newX, float currentX, float newY);
bool checkSpriteCollisionWithTileY(float newY, float currentY, float newX);
bool checkSpriteCollisionWithSprite(float s1x, float s1y, float s2x, float s2y);
bool isVisible(int x0, int y0, int x1, int y1);
bool isWalkable(int x, int y);
bool nearTile(TileTypes tile);
void unstuckEnemy(Enemy& enemy);

// ─────────────────────────────────────────────────────────────────────────────
// Dungeon corridor carving  (used by Dungeon.cpp, lives here to avoid circular)
// ─────────────────────────────────────────────────────────────────────────────
void carveHorizontalCorridor(int x1, int x2, int y);
void carveVerticalCorridor(int y1, int y2, int x);
void getEdgeTowards(const Room& from, const Room& to, int& outX, int& outY);
void swap(int& a, int& b);
int  countWalls(int x, int y);

// ─────────────────────────────────────────────────────────────────────────────
// Screen shake
// ─────────────────────────────────────────────────────────────────────────────
void updateScreenShake();
void triggerScreenShake(int duration, int intensity);

// ─────────────────────────────────────────────────────────────────────────────
// UI rendering helpers
// ─────────────────────────────────────────────────────────────────────────────
void renderUI();
void updateAnimations();
void drawWrappedText(int x, int y, int maxWidth, const char* text);

// ─────────────────────────────────────────────────────────────────────────────
// Name / seed generation
// ─────────────────────────────────────────────────────────────────────────────
void     trainFemaleMarkov();
void     generateFemaleName(char* name, size_t nameSize);
uint32_t generateRandomSeed();

// ─────────────────────────────────────────────────────────────────────────────
// Death check
// ─────────────────────────────────────────────────────────────────────────────
void checkIfDeadFrom(const char* cause);

// ─────────────────────────────────────────────────────────────────────────────
// Riddle UI
// ─────────────────────────────────────────────────────────────────────────────
void generateRiddleUI();

// ─────────────────────────────────────────────────────────────────────────────
// Save / load  (thin wrappers that marshal g_state ↔ SaveData)
// ─────────────────────────────────────────────────────────────────────────────
void trySaveGame();
void tryLoadGame();

#endif // HELPERFUNCTIONS_H