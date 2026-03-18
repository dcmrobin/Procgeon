#ifndef PLAYER_H
#define PLAYER_H

#include "Common.h"
#include "GameState.h"
#include "Item.h"
#include "Puzzles.h"

// ─────────────────────────────────────────────────────────────────────────────
// Player.h
//
// All player state variables live in GameState.h / GameState.cpp.
// This header only declares the player subsystem functions.
// ─────────────────────────────────────────────────────────────────────────────

// ── Rendering ────────────────────────────────────────────────────────────
void renderPlayer();

// ── Input & movement ─────────────────────────────────────────────────────
void handleInput();
void startCarryingDamsel(bool resetPickupTimer);

// ── UI screens ────────────────────────────────────────────────────────────
void handlePauseScreen();

// ── Per-tick effects ──────────────────────────────────────────────────────
void handleHungerAndEffects();
void handleRingEffects();

// ── Dialogue ──────────────────────────────────────────────────────────────
void handleDialogue();
void playDamselSFX(const char* tone);

// ── Riddles ───────────────────────────────────────────────────────────────
void handleRiddles();

// ── Chest / puzzle gate ───────────────────────────────────────────────────
void OpenChest(int cy, int cx, int dx);
void finishPendingChest(bool success);

#endif // PLAYER_H