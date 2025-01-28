#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <Arduino.h>

#define seedPin A0

uint32_t generateRandomSeed();
void carveHorizontalCorridor(int x1, int x2, int y);
void carveVerticalCorridor(int y1, int y2, int x);
void swap(int &a, int &b);
int countWalls(int x, int y);

#endif