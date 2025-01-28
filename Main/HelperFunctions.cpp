#include "HelperFunctions.h"

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