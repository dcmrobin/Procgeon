#include "HelperFunctions.h"
#include "Dungeon.h"

uint32_t generateRandomSeed()
{
  uint8_t  seedBitValue  = 0;
  uint8_t  seedByteValue = 0;
  uint32_t seedWordValue = 0;
 
  for (uint8_t wordShift = 0; wordShift < 4; wordShift++)     // 4 bytes in a 32 bit word
  {
    for (uint8_t byteShift = 0; byteShift < 8; byteShift++)   // 8 bits in a byte
    {
      for (uint8_t bitSum = 0; bitSum <= 8; bitSum++)         // 8 samples of analog pin
      {
        seedBitValue = seedBitValue + (analogRead(seedPin) & 0x01);                // Flip the coin eight times, adding the results together
      }
      delay(1);                                                                    // Delay a single millisecond to allow the pin to fluctuate
      seedByteValue = seedByteValue | ((seedBitValue & 0x01) << byteShift);        // Build a stack of eight flipped coins
      seedBitValue = 0;                                                            // Clear out the previous coin value
    }
    seedWordValue = seedWordValue | (uint32_t)seedByteValue << (8 * wordShift);    // Build a stack of four sets of 8 coins (shifting right creates a larger number so cast to 32bit)
    seedByteValue = 0;                                                             // Clear out the previous stack value
  }
  return (seedWordValue);
 
}

// Carve a horizontal corridor
void carveHorizontalCorridor(int x1, int x2, int y) {
  if (x1 > x2) swap(x1, x2);
  for (int x = x1; x <= x2; x++) {
    dungeonMap[y][x] = 1; // Floor
  }
}

// Carve a vertical corridor
void carveVerticalCorridor(int y1, int y2, int x) {
  if (y1 > y2) swap(y1, y2);
  for (int y = y1; y <= y2; y++) {
    dungeonMap[y][x] = 1; // Floor
  }
}

// Utility function to swap values
void swap(int &a, int &b) {
  int temp = a;
  a = b;
  b = temp;
}

// Count surrounding walls for smoothing
int countWalls(int x, int y) {
  int wallCount = 0;
  for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {
      if (dx != 0 || dy != 0) {
        if (dungeonMap[y + dy][x + dx] == 2) {
          wallCount++;
        }
      }
    }
  }
  return wallCount;
}