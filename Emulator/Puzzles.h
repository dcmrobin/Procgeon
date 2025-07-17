#ifndef PUZZLES_H
#define PUZZLES_H

#define PICROSS_SIZE 5
#define LIGHTSOUT_SIZE 5

// Picross puzzle state
extern bool picrossSolution[PICROSS_SIZE][PICROSS_SIZE];
extern bool picrossPlayerGrid[PICROSS_SIZE][PICROSS_SIZE];

//extern int cx;
//extern int cy;
//extern int dx;
//extern bool solved;

void resetPicrossPuzzle();
void generatePicrossPuzzle();
void drawPicrossPuzzle();
void handlePicrossInput();
bool isPicrossSolved();
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
void updateLightsOutPuzzle();

// Main puzzle launcher
void startRandomPuzzle(int n_cy, int n_cx, int n_dx);

#endif // PUZZLES_H