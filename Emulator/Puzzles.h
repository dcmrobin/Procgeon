#ifndef PUZZLES_H
#define PUZZLES_H

#include "Translation.h"

#define PICROSS_SIZE 5
#define LIGHTSOUT_SIZE 5

// Picross puzzle state
extern bool picrossSolution[PICROSS_SIZE][PICROSS_SIZE];
extern bool picrossPlayerGrid[PICROSS_SIZE][PICROSS_SIZE];

void resetPicrossPuzzle();
void generatePicrossPuzzle();
void drawPicrossPuzzle();
void handlePicrossInput();
bool isPicrossSolved();
bool launchPicrossPuzzle();
// New non-blocking API
void startPicrossPuzzle();
void updatePicrossPuzzle();

// Lights Out puzzle state
extern bool lightsOutGrid[LIGHTSOUT_SIZE][LIGHTSOUT_SIZE];
extern int lightsOutCursorX;
extern int lightsOutCursorY;

void resetLightsOutPuzzle();
void generateLightsOutPuzzle();
void drawLightsOutPuzzle();
void handleLightsOutInput();
bool isLightsOutSolved();
bool launchLightsOutPuzzle();
// New non-blocking API
void startLightsOutPuzzle();
void updateLightsOutPuzzle();

// Main puzzle launcher
bool launchRandomPuzzle();

// Status reported after non-blocking puzzle finishes
extern bool puzzleFinished; // set true when a started puzzle completes or is cancelled
extern bool puzzleSuccess;  // true if the puzzle was solved, false if cancelled/failed

#endif // PUZZLES_H