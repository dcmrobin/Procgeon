#include "HelperFunctions.h"

ButtonStates buttons = {false};

// Add action selection tracking
int selectedActionIndex = 0; // 0 = Use, 1 = Drop, 2 = Info

UIState currentUIState = UI_NORMAL; // Current UI state

bool statusScreen = false;

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
int predictXtile(float x) {
  return (int)(x + 0.5f); // Always round to the nearest integer
}
int predictYtile(float y) {
  return (int)(y + 0.5f); // Always round to the nearest integer
}
bool checkSpriteCollisionWithTileX(float newX, float currentX, float newY) {
    int ptx = predictXtile(newX);
    int cty = round(newY);
    bool xValid = (newX >= 0 && newX < mapWidth && dungeonMap[cty][ptx] == 1);
    if (!xValid) {
        newX = currentX;
    }
    return !xValid;
}
bool checkSpriteCollisionWithTileY(float newY, float currentY, float newX) {
    int pty = predictYtile(newY);
    int ctx = round(newX);
    bool yValid = (newY >= 0 && newY < mapHeight && dungeonMap[pty][ctx] == 1);
    if (!yValid) {
        newY = currentY;
    }
    return !yValid;
}
bool checkSpriteCollisionWithSprite(float sprite1X, float sprite1Y, float sprite2X, float sprite2Y) {
  // Use predictXtile/predictYtile for consistent rounding
  int tile1X = predictXtile(sprite1X);
  int tile1Y = predictYtile(sprite1Y);
  int tile2X = predictXtile(sprite2X);
  int tile2Y = predictYtile(sprite2Y);
  return tile1X == tile2X && tile1Y == tile2Y;
}

void updateButtonStates() {
  // Save previous states
  buttons.upPressedPrev = buttons.upPressed;
  buttons.downPressedPrev = buttons.downPressed;
  buttons.aPressedPrev = buttons.aPressed;
  buttons.bPressedPrev = buttons.bPressed;

  // Read current states
  buttons.upPressed = !digitalRead(BUTTON_UP_PIN);
  buttons.downPressed = !digitalRead(BUTTON_DOWN_PIN);
  buttons.aPressed = !digitalRead(BUTTON_A_PIN);
  buttons.bPressed = !digitalRead(BUTTON_B_PIN);
}

void handleUIStateTransitions(bool hasMap) {
  if (buttons.aPressed && !buttons.aPressedPrev) {
    switch (currentUIState) {
      case UI_NORMAL: 
        if (!statusScreen) currentUIState = UI_INVENTORY;
        break;
      case UI_INVENTORY: 
        currentUIState = hasMap ? UI_MINIMAP : UI_NORMAL; 
        break;
      case UI_MINIMAP: 
        currentUIState = UI_NORMAL; 
        break;
      case UI_ITEM_ACTION: 
        currentUIState = UI_INVENTORY;
        break;
      case UI_ITEM_INFO: 
        currentUIState = UI_INVENTORY;
        break;
      case UI_ITEM_RESULT: 
        currentUIState = UI_NORMAL;
        break;
    }
  }
}

int blobanimcounter = 0;
int damselanimcounter = 0;
void updateAnimations() {
  blobanimcounter += 1;
  if (blobanimcounter >= 20) {
    blobSprite = blobSprite == blobSpriteFrame1 ? blobSpriteFrame2 : blobSpriteFrame1;
    blobanimcounter = 0;
  }
  
  damselanimcounter += 1;
  if (damselanimcounter >= random(50, 90)) {
    damselSprite = damsel[0].dead ? damselSpriteDead : damselSprite;
    damselanimcounter = 0;
  }
}