#ifndef PUZZLES_H
#define PUZZLES_H

#include "Common.h"
#include "GameState.h"

// ─────────────────────────────────────────────────────────────────────────────
// Puzzle state (defined in Puzzles.cpp)
// ─────────────────────────────────────────────────────────────────────────────
extern bool picrossSolution[PICROSS_SIZE][PICROSS_SIZE];
extern bool picrossPlayerGrid[PICROSS_SIZE][PICROSS_SIZE];
extern bool lightsOutGrid[LIGHTSOUT_SIZE][LIGHTSOUT_SIZE];
extern int  lightsOutCursorX;
extern int  lightsOutCursorY;

// Set true when a non-blocking puzzle finishes; puzzleSuccess indicates outcome
extern bool puzzleFinished;
extern bool puzzleSuccess;

// ─────────────────────────────────────────────────────────────────────────────
// Picross
// ─────────────────────────────────────────────────────────────────────────────
void resetPicrossPuzzle();
void generatePicrossPuzzle();
bool launchPicrossPuzzle();    // blocking launch (legacy)
void startPicrossPuzzle();     // non-blocking start
void updatePicrossPuzzle();    // non-blocking per-frame update

// ─────────────────────────────────────────────────────────────────────────────
// Lights Out
// ─────────────────────────────────────────────────────────────────────────────
void resetLightsOutPuzzle();
void generateLightsOutPuzzle();
bool launchLightsOutPuzzle();  // blocking launch (legacy)
void startLightsOutPuzzle();   // non-blocking start
void updateLightsOutPuzzle();  // non-blocking per-frame update

// ─────────────────────────────────────────────────────────────────────────────
// Random launcher
// ─────────────────────────────────────────────────────────────────────────────
bool launchRandomPuzzle();

#endif // PUZZLES_H