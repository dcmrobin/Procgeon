#include "HelperFunctions.h"

U8G2_SH1107_PIMORONI_128X128_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RST);

ButtonStates buttons = {false};

// Add action selection tracking
int selectedActionIndex = 0; // 0 = Use, 1 = Drop, 2 = Info

UIState currentUIState = UI_NORMAL; // Current UI state

bool statusScreen = false;

const int viewportWidth = SCREEN_WIDTH / tileSize;
const int viewportHeight = SCREEN_HEIGHT / tileSize - 2;

float offsetX = 0;
float offsetY = 0;

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

void drawWrappedText(const char *text, int x, int y, int maxWidth, int lineHeight) {
  const char *wordStart = text;
  char lineBuffer[256] = {0};  // Buffer for building a line
  int lineBufferLen = 0;
  
  while (*wordStart) {
    // Find the next space or end of string
    const char *wordEnd = wordStart;
    while (*wordEnd && *wordEnd != ' ') {
      wordEnd++;
    }
    
    // Extract the word
    int wordLen = wordEnd - wordStart;
    char word[64] = {0}; // Assumes words are less than 64 characters
    strncpy(word, wordStart, wordLen);
    word[wordLen] = '\0';
    
    // Determine the width of the current line plus a space (if needed) and the new word
    //int testLen = lineBufferLen > 0 ? lineBufferLen + 1 + wordLen : wordLen;
    
    // Create a temporary string to measure
    char testLine[256] = {0};
    if (lineBufferLen > 0) {
      snprintf(testLine, sizeof(testLine), "%s %s", lineBuffer, word);
    } else {
      snprintf(testLine, sizeof(testLine), "%s", word);
    }
    
    int textWidth = u8g2.getStrWidth(testLine);
    
    // If the line is too long, draw the current line and start a new one.
    if (textWidth > maxWidth && lineBufferLen > 0) {
      u8g2.drawStr(x, y, lineBuffer);
      y += lineHeight;
      lineBuffer[0] = '\0';  // Reset the line buffer
      lineBufferLen = 0;
      
      // If the word itself is longer than maxWidth, you might need to break the word further.
      // For now, we'll just put the long word on its own line.
    }
    
    // Append the word to the line buffer
    if (lineBufferLen > 0) {
      strncat(lineBuffer, " ", sizeof(lineBuffer) - strlen(lineBuffer) - 1);
      lineBufferLen++;
    }
    strncat(lineBuffer, word, sizeof(lineBuffer) - strlen(lineBuffer) - 1);
    lineBufferLen = strlen(lineBuffer);
    
    // Skip any spaces in the source text
    while (*wordEnd == ' ') {
      wordEnd++;
    }
    wordStart = wordEnd;
  }
  
  // Draw any remaining text in the line buffer
  if (lineBufferLen > 0) {
    u8g2.drawStr(x, y, lineBuffer);
  }
}

void renderUI(int playerHP, int level, bool hasMap) { 
  char HP[4];
  char Lvl[7];
  snprintf(HP, sizeof(HP), "%d", playerHP); // Convert playerHP to a string
  snprintf(Lvl, sizeof(Lvl), "%d", level);
  
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(5, 123, "HP:");
  u8g2.drawStr(20, 123, HP);
  u8g2.drawStr(40, 123, "LVL:");
  u8g2.drawStr(60, 123, Lvl);
  u8g2.drawFrame(0, 113, SCREEN_WIDTH, 15);
  if (hasMap) {
    u8g2.drawXBM(70, 115, 8, 8, mapSprite);
  }
}

bool isVisible(int x0, int y0, int x1, int y1) {
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  while (true) {
    // Check bounds
    if (x0 < 0 || x0 >= mapWidth || y0 < 0 || y0 >= mapHeight) return false;
    
    // Check if tile blocks visibility
    int tile = dungeonMap[y0][x0];
    if (tile == 2 || tile == 3) return false; // Walls or bars
    
    // Reached target tile
    if (x0 == x1 && y0 == y1) break;
    
    // Move to next tile
    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
  return true;
}