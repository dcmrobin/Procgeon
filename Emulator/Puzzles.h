#ifndef PUZZLES_H
#define PUZZLES_H

#include "Translation.h"

#define PICROSS_SIZE 5
#define LIGHTSOUT_SIZE 5

extern bool picrossSolution[PICROSS_SIZE][PICROSS_SIZE];
extern bool picrossPlayerGrid[PICROSS_SIZE][PICROSS_SIZE];
extern bool lightsOutGrid[LIGHTSOUT_SIZE][LIGHTSOUT_SIZE];

void resetPicrossPuzzle();
void generatePicrossPuzzle();
void drawPicrossPuzzle();
void handlePicrossInput();
bool isPicrossSolved();
bool updatePicrossPuzzle(); // Returns true when solved

void resetLightsOutPuzzle();
void generateLightsOutPuzzle();
void drawLightsOutPuzzle();
void handleLightsOutInput();
bool isLightsOutSolved();
bool updateLightsOutPuzzle(); // Returns true when solved

bool updateRandomPuzzle(); // Returns true when solved
void launchRandomPuzzle(int cy, int cx, int dx);

#endif