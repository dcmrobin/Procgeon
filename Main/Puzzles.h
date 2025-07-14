#ifndef PUZZLES_H
#define PUZZLES_H

#include <Arduino.h>

#define PICROSS_SIZE 5

// Puzzle state
extern bool picrossSolution[PICROSS_SIZE][PICROSS_SIZE];
extern bool picrossPlayerGrid[PICROSS_SIZE][PICROSS_SIZE];

void resetPicrossPuzzle();
void generatePicrossPuzzle();
void drawPicrossPuzzle();
void handlePicrossInput();
bool isPicrossSolved();
bool launchPicrossPuzzle();

#endif // PUZZLES_H
