#include "HelperFunctions.h"
#include "Player.h"

Adafruit_SSD1327 display(128, 128, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

ButtonStates buttons = {false};

// Add action selection tracking
int selectedActionIndex = 0; // 0 = Use, 1 = Drop, 2 = Info

UIState currentUIState = UI_NORMAL; // Current UI state

bool statusScreen = false;

const int viewportWidth = SCREEN_WIDTH / tileSize;
const int viewportHeight = SCREEN_HEIGHT / tileSize - 2;

float offsetX = 0;
float offsetY = 0;

const float scrollSpeed = 0.25f;

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
    dungeonMap[y][x] = Floor;
  }
}
// Carve a vertical corridor
void carveVerticalCorridor(int y1, int y2, int x) {
  if (y1 > y2) swap(y1, y2);
  for (int y = y1; y <= y2; y++) {
    dungeonMap[y][x] = Floor;
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
        if (dungeonMap[y + dy][x + dx] == Wall) {
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
    bool xValid = (newX >= 0 && newX < mapWidth && dungeonMap[cty][ptx] == Floor);
    if (!xValid) {
        newX = currentX;
    }
    return !xValid;
}
bool checkSpriteCollisionWithTileY(float newY, float currentY, float newX) {
    int pty = predictYtile(newY);
    int ctx = round(newX);
    bool yValid = (newY >= 0 && newY < mapHeight && dungeonMap[pty][ctx] == Floor);
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
  buttons.leftPressedPrev = buttons.leftPressed;
  buttons.rightPressedPrev = buttons.rightPressed;
  buttons.startPressedPrev = buttons.startPressed;
  buttons.selectPressedPrev = buttons.selectPressed;

  // Read current states
  buttons.upPressed = !digitalRead(BUTTON_UP_PIN);
  buttons.downPressed = !digitalRead(BUTTON_DOWN_PIN);
  buttons.aPressed = !digitalRead(BUTTON_A_PIN);
  buttons.bPressed = !digitalRead(BUTTON_B_PIN);
  buttons.leftPressed = !digitalRead(BUTTON_LEFT_PIN);
  buttons.rightPressed = !digitalRead(BUTTON_RIGHT_PIN);
  buttons.startPressed = !digitalRead(BUTTON_START_PIN);
  buttons.selectPressed = !digitalRead(BUTTON_SELECT_PIN);
}

void handleUIStateTransitions() {
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

void renderUI() {
    char HP[4];
    char Dngn[7];
    snprintf(HP, sizeof(HP), "%d", playerHP);
    snprintf(Dngn, sizeof(Dngn), "%d", dungeon);

    u8g2_for_adafruit_gfx.setFont(u8g2_font_profont12_tr);
    display.setCursor(5, 117);
    display.print("HP:");
    display.setCursor(21, 117);
    display.print(HP);
    display.setCursor(46, 117);
    display.print("DUNGEON:");
    display.setCursor(92, 117);
    display.print(Dngn);
    display.drawRect(0, 113, SCREEN_WIDTH, 15, SSD1327_WHITE);
    if (hasMap) {
        display.drawBitmap(100, 116, mapSprite, 8, 8, SSD1327_WHITE);
    }
}

bool isVisible(int x0, int y0, int x1, int y1) {
  int dx = abs(x1 - x0);
  int dy = abs(y1 - y0);
  int sx = (x0 < x1) ? 1 : -1;
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx - dy;

  while (true) {
    // Check bounds first
    if (x0 < 0 || x0 >= mapWidth || y0 < 0 || y0 >= mapHeight) return false;

    // **If we've reached the target tile, exit the loop**
    if (x0 == x1 && y0 == y1)
      break;

    // Now check for obstruction
    TileTypes tile = dungeonMap[y0][x0];
    if (tile == Wall)
      return false;

    // Move to the next tile along the line
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