#include "HelperFunctions.h"
#include "Player.h"
#include "GameAudio.h"
#include "Inventory.h"
#include "SaveLogic.h"
#include "Item.h"
#include <string.h>

#define MAX_LETTERS 26
#define NAME_BUFFER_SIZE 10  // Maximum length for generated names

Adafruit_SSD1327 display(128, 128, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

ButtonStates buttons = {false};

// Add action selection tracking
int selectedActionIndex = 0; // 0 = Use, 1 = Drop, 2 = Info

UIState currentUIState = UI_NORMAL; // Current UI state

SaveData saveData = {0};

bool statusScreen = false;
bool finalStatusScreen = false;
bool showDeathScreen = false;
bool credits = false;

const int viewportWidth = SCREEN_WIDTH / tileSize;
const int viewportHeight = SCREEN_HEIGHT / tileSize - 2;

float offsetX = 0;
float offsetY = 0;

int shakeDuration = 0;   // How many frames to shake for
int shakeIntensity = 1;  // How strong the shake is

uint32_t worldSeed = 0;

const float scrollSpeed = 0.25f;

// Global variable for the current riddle
GeneratedRiddle currentRiddle;
int selectedRiddleOption = 0;  // Tracks the current selection when scrolling
bool riddleGenerated = false;  // Flag to generate only once per riddle

// Define a larger list of possible answers with their descriptive attributes
RiddleAnswer possibleAnswers[] = {
    {"sea",      {"boundless", "whispering", "enigmatic", "murmuring"}},
    {"fire",     {"burning", "flickering", "transient", "intense"}},
    {"shadow",   {"elusive", "fleeting", "whispering", "ephemeral"}},
    {"cloud",    {"drifting", "evanescent", "vague", "mysterious"}},
    {"wind",     {"invisible", "restless", "unseen", "quiet"}},
    {"time",     {"slipping", "silent", "elusive", "everflowing"}},
    {"mountain", {"stolid", "ancient", "unmoving", "enduring"}},
    {"river",    {"wandering", "serene", "meandering", "subtle"}},
    {"star",     {"distant", "silent", "celestial", "untold"}},
    {"tree",     {"rooted", "whispering", "quiet", "timeless"}}
};
const int numAnswers = sizeof(possibleAnswers) / sizeof(possibleAnswers[0]);

// Define many riddle templates (using %s for attribute insertion)
const char* templates[] = {
    "I am always %s, yet sometimes %s. What am I?",
    "I can be %s and also %s. What could I be?",
    "Often %s, occasionally %s. Who am I?",
    "I embody %s and reveal %s. Can you name me?",
    "Known for being %s, and sometimes %s. Guess what I am?",
    "I am described as %s, but also %s. What am I?",
    "Many say I am %s, yet I can be %s too. Who am I?",
    "I move with %s and hide in %s. What might I be?",
    "I am both %s and %s. Who might I be?",
    "I appear %s, and sometimes I feel %s. What is my name?",
    "I am often seen as %s, but I can turn %s. Who am I?",
    "Some say I'm %s, while others call me %s. What am I?"
};
const int numTemplates = sizeof(templates) / sizeof(templates[0]);

// Transition table for letters [a-z]
int femaleTransition[MAX_LETTERS][MAX_LETTERS];

// Sample female names for training
const char* sampleFemaleNames[] = {"Liora", "Lucy", "Ruby", "Talitha", "Mary", "Sarah", "Salina", "Olivia", "Evelyn", "Valerie", "Jenny", "Eva", "Luna", "Isabella", "Maria", "Arwen", "Sophie", "Felicity", "Rebecca", "Julia", "Rebecca"};
const int sampleFemaleNamesCount = sizeof(sampleFemaleNames) / sizeof(sampleFemaleNames[0]);

// Helper function to shuffle an array of integers
void shuffleArray(int arr[], int n) {
  for (int i = n - 1; i > 0; i--) {
    int j = random(i + 1);
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
  }
}

void generateRiddleUI() {
  // Pick a random answer from the list as the correct answer.
  int answerIndex = random(numAnswers);
  RiddleAnswer chosen = possibleAnswers[answerIndex];

  // Pick two distinct attributes from the chosen answer.
  int index1 = random(4);
  int index2 = random(4);
  while (index2 == index1) {
    index2 = random(4);
  }
  const char* attr1 = chosen.attributes[index1];
  const char* attr2 = chosen.attributes[index2];

  // Choose a random template and format the riddle string.
  int templateIndex = random(numTemplates);
  char riddleBuffer[128];  // Buffer for the formatted riddle.
  snprintf(riddleBuffer, sizeof(riddleBuffer), templates[templateIndex], attr1, attr2);
  snprintf(currentRiddle.riddle, sizeof(currentRiddle.riddle), "%s", riddleBuffer);

  // Prepare a list of indices for the answer options.
  const int totalOptions = 4;
  int optionIndices[totalOptions];
  optionIndices[0] = answerIndex;  // The correct answer.
  int count = 1;
  while (count < totalOptions) {
    int decoy = random(numAnswers);
    bool unique = true;
    if (decoy == answerIndex) {
      unique = false;
    } else {
      for (int i = 0; i < count; i++) {
        if (optionIndices[i] == decoy) {
          unique = false;
          break;
        }
      }
    }
    if (unique) {
      optionIndices[count] = decoy;
      count++;
    }
  }

  // Shuffle the options so the correct answer isn’t always first.
  shuffleArray(optionIndices, totalOptions);

  // Fill in the answer options and record the index of the correct answer.
  for (int i = 0; i < totalOptions; i++) {
    snprintf(currentRiddle.options[i], sizeof(currentRiddle.options[i]), "%s", possibleAnswers[optionIndices[i]].word);
    if (optionIndices[i] == answerIndex) {
      currentRiddle.correctOption = i;
    }
  }
  // Reset selection and flag so we don’t regenerate the riddle repeatedly.
  selectedRiddleOption = 0;
  riddleGenerated = true;
}

// --- Train the Markov model ---
void trainFemaleMarkov() {
  // Initialize the transition table to zero
  for (int i = 0; i < MAX_LETTERS; i++) {
    for (int j = 0; j < MAX_LETTERS; j++) {
      femaleTransition[i][j] = 0;
    }
  }

  // Build transitions from each sample name
  for (int i = 0; i < sampleFemaleNamesCount; i++) {
    const char* name = sampleFemaleNames[i];
    int len = strlen(name);
    for (int j = 0; j < len - 1; j++) {
      char current = tolower(name[j]);
      char next = tolower(name[j + 1]);
      if (current >= 'a' && current <= 'z' && next >= 'a' && next <= 'z') {
        femaleTransition[current - 'a'][next - 'a']++;
      }
    }
  }
}

// --- Generate a female name using the Markov chain ---
void generateFemaleName(char *name, size_t nameSize) {
  char tempName[NAME_BUFFER_SIZE + 1];
  // Start with a random letter (a–z)
  int startLetter = random(0, MAX_LETTERS);
  tempName[0] = 'a' + startLetter;
  
  // Randomly choose a length between 4 and NAME_BUFFER_SIZE
  int length = random(4, NAME_BUFFER_SIZE);
  
  for (int i = 1; i < length; i++) {
    int prev = tempName[i - 1] - 'a';
    
    // Calculate the total weight from the transition table for the previous letter
    int total = 0;
    for (int j = 0; j < MAX_LETTERS; j++) {
      total += femaleTransition[prev][j];
    }
    
    // Default next letter is random
    int nextLetter = random(0, MAX_LETTERS);
    
    // If there is training data, select a weighted next letter
    if (total > 0) {
      int rnd = random(0, total);
      int cumulative = 0;
      for (int j = 0; j < MAX_LETTERS; j++) {
        cumulative += femaleTransition[prev][j];
        if (rnd < cumulative) {
          nextLetter = j;
          break;
        }
      }
    }
    tempName[i] = 'a' + nextLetter;
  }
  tempName[length] = '\0';
  
  // Capitalize the first letter
  tempName[0] = toupper(tempName[0]);
  
  bool hasBadLetter = false;
  for (int i = 0; i < length; i++) {
    if (tempName[i] == 'z' || tempName[i] == 'Z' || tempName[i] == 'x' || tempName[i] == 'X' || tempName[i] == 'q' || tempName[i] == 'Q') {
      hasBadLetter = true;
    }
  }

  if (!hasBadLetter) {
    snprintf(name, nameSize, "%s", tempName);
  } else {
    snprintf(name, nameSize, "%s", sampleFemaleNames[random(0, sizeof(sampleFemaleNames) / sizeof(sampleFemaleNames[0]))]);
  }
}

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

void carveVerticalCorridor(int y1, int y2, int x) {
  if (y1 > y2) swap(y1, y2);
  for (int y = y1; y <= y2; y++) {
    dungeonMap[y][x] = Floor;
  }
}
void getEdgeTowards(const Room& from, const Room& to, int& outX, int& outY) {
  int dx = (to.x + to.width / 2) - (from.x + from.width / 2);
  int dy = (to.y + to.height / 2) - (from.y + from.height / 2);

  if (abs(dx) > abs(dy)) {
    // Horizontal edge
    outY = from.y + random(1, from.height - 2);
    if (dx > 0) {
      // Exit right
      outX = from.x + from.width - 1;
    } else {
      // Exit left
      outX = from.x;
    }
  } else {
    // Vertical edge
    outX = from.x + random(1, from.width - 2);
    if (dy > 0) {
      // Exit bottom
      outY = from.y + from.height - 1;
    } else {
      // Exit top
      outY = from.y;
    }
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
    bool xValid = (newX >= 0 && newX < mapWidth && dungeonMap[cty][ptx] != Wall && dungeonMap[cty][ptx] != Bars && dungeonMap[cty][ptx] != ChestTile);
    if (!xValid) {
        newX = currentX;
    }
    return !xValid;
}
bool checkSpriteCollisionWithTileY(float newY, float currentY, float newX) {
    int pty = predictYtile(newY);
    int ctx = round(newX);
    bool yValid = (newY >= 0 && newY < mapHeight && dungeonMap[pty][ctx] != Wall && dungeonMap[pty][ctx] != Bars && dungeonMap[pty][ctx] != ChestTile);
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

  // Read current states
  buttons.upPressed = !digitalRead(BUTTON_UP_PIN);
  buttons.downPressed = !digitalRead(BUTTON_DOWN_PIN);
  buttons.aPressed = !digitalRead(BUTTON_A_PIN);
  buttons.bPressed = !digitalRead(BUTTON_B_PIN);
  buttons.leftPressed = !digitalRead(BUTTON_LEFT_PIN);
  buttons.rightPressed = !digitalRead(BUTTON_RIGHT_PIN);
  buttons.startPressed = !digitalRead(BUTTON_START_PIN);
}

void handleUIStateTransitions() {
  if (buttons.aPressed && !buttons.aPressedPrev) {
    switch (currentUIState) {
      case UI_NORMAL: 
        if (!statusScreen) {
          if (!showDialogue) {
            currentUIState = UI_INVENTORY;
            playRawSFX(12);
          } else {
            showDialogue = false;
          }
        }
        break;
      case UI_INVENTORY: 
        currentUIState = hasMap ? UI_MINIMAP : UI_NORMAL; 
        if (!hasMap) {
          playRawSFX(13);
        }
        if (identifyingItem) {
          // Waste the scroll
          if (identifyScrollPage >= 0 && identifyScrollIndex >= 0) {
            inventoryPages[identifyScrollPage].items[identifyScrollIndex] = { Null, PotionCategory, "Empty"};
            inventoryPages[identifyScrollPage].itemCount--;
          }
          identifyingItem = false;
          identifyScrollPage = -1;
          identifyScrollIndex = -1;
        }
        break;
      case UI_MINIMAP: 
        playRawSFX(13);
        currentUIState = UI_NORMAL; 
        break;
      case UI_ITEM_ACTION: 
        playRawSFX(12);
        currentUIState = UI_INVENTORY;
        break;
      case UI_ITEM_INFO: 
        playRawSFX(12);
        currentUIState = UI_INVENTORY;
        break;
      case UI_ITEM_RESULT: 
        playRawSFX(12);
        currentUIState = UI_NORMAL;
        break;
      case UI_PAUSE: 
        currentUIState = UI_PAUSE;
        break;
      case UI_RIDDLE:
        currentUIState = UI_RIDDLE;
        break;
      case UI_SPLASH:
        currentUIState = UI_SPLASH;
        break;
      case UI_INTRO:
        currentUIState = UI_INTRO;
        break;
    }
  } else if (buttons.startPressed && !buttons.startPressedPrev) {
    playRawSFX(9);
    if (currentUIState == UI_SPLASH) {
      playWav1.stop();
      shouldRestartGame = true;
    }
    currentUIState = currentUIState == UI_PAUSE || currentUIState == UI_SPLASH ? UI_NORMAL : currentUIState == UI_NORMAL ? UI_PAUSE : currentUIState;
  }
}

int damselanimcounter = 0;
void updateAnimations() {
  
  damselanimcounter += 1;
  if (damselanimcounter >= random(50, 90)) {
    damselSprite = damsel[0].dead ? damselSpriteDead : damselSprite;
    damselanimcounter = 0;
  }

  // Enemy animation update logic
  static int frameIndex[30] = {0};
  static int frameTimer[30] = {0};
  for (int i = 0; i < maxEnemies; ++i) {
    Enemy& e = enemies[i];
    int animLength = 1;
    const Frame* anim = nullptr;
    if (strcmp(e.name, "blob") == 0) {
      anim = blobAnimation;
      animLength = blobAnimationLength;
    } else if (strcmp(e.name, "teleporter") == 0) {
      anim = teleporterAnimation;
      animLength = teleporterAnimationLength;
    } else if (strcmp(e.name, "batguy") == 0) {
      anim = batguyAnimation;
      animLength = batguyAnimationLength;
    } else if (strcmp(e.name, "shooter") == 0) {
      anim = shooterAnimation;
      animLength = shooterAnimationLength;
    } else if (strcmp(e.name, "clock") == 0) {
      anim = clockAnimation;
      animLength = clockAnimationLength;
    } else if (strcmp(e.name, "jukebox") == 0) {
      anim = jukeboxAnimation;
      animLength = jukeboxAnimationLength;
    } else if (strcmp(e.name, "boss") == 0) {
      if (bossState == Idle) {
        if (playerX < enemies[0].x) {
          anim = bossIdleAnimationFlipped;
          animLength = bossIdleAnimationLength;
        } else if (playerX >= enemies[0].x) {
          anim = bossIdleAnimation;
          animLength = bossIdleAnimationLength;
        }
      } else if (bossState == Floating) {
        anim = bossFightAnimation;
        animLength = bossFightAnimationLength;
      } else if (bossState == Shooting) {
        if (playerX < enemies[0].x) {
          anim = bossIdleAnimationFlipped;
          animLength = bossIdleAnimationLength;
        } else if (playerX >= enemies[0].x) {
          anim = bossIdleAnimation;
          animLength = bossIdleAnimationLength;
        }
      } else if (bossState == Summoning) {
        anim = bossFightAnimation;
        animLength = bossFightAnimationLength;
      } else if (bossState == Enraged) {
        anim = bossFightAnimation;
        animLength = bossFightAnimationLength;
      } else if (bossState == Beaten) {
        anim = bossBeatenAnimation;
        animLength = bossBeatenAnimationLength;
      }
    }
    if (anim) {
      frameTimer[i]++;
      if (frameTimer[i] >= anim[frameIndex[i]].length) {
        if (anim != bossBeatenAnimation) {
          frameTimer[i] = 0;
          frameIndex[i] = (frameIndex[i] + 1) % animLength;
          e.sprite = anim[frameIndex[i]].frame;
        } else {
          // Boss beaten animation does not loop
          if (frameIndex[i] < animLength - 1) {
            frameTimer[i] = 0;
            frameIndex[i]++;
            e.sprite = anim[frameIndex[i]].frame;
          }
        }
      }
    }
  }
}

int blinkTick = 0;
int textColor = 3;
void renderUI() {
  char HP[4];
  char FOOD[7];
  snprintf(HP, sizeof(HP), "%d", playerHP);
  snprintf(FOOD, sizeof(FOOD), "%d", playerFood);

  display.setTextColor(15);
  display.setTextSize(1);
  display.setCursor(5, 117);
  display.print("HP:");
  display.setCursor(21, 117);
  display.print(HP);
  display.setTextColor(textColor);

  if (starving) {
    blinkTick += 1;
    if (blinkTick >= 35) {
      textColor = (textColor == 15) ? 3 : 15; // Toggle text color
      blinkTick = 0;
    }
  } else {
    textColor = 15; // Default color when not starving
  }

  display.setTextColor(textColor);

  display.setCursor(46, 117);
  display.print("FOOD:");
  display.setCursor(74, 117);
  display.print(FOOD);
  display.drawRect(0, 113, SCREEN_WIDTH, 15, SSD1327_WHITE);
  if (hasMap) {
    display.drawBitmap(100, 116, mapSprite, 8, 8, SSD1327_WHITE);
  }
  if (speeding || paralyzed) {
    display.drawBitmap(109, 116, fastbootSprite, 8, 8, SSD1327_WHITE);
  }
  if (seeAll || blinded) {
    display.drawBitmap(118, 116, eyeSprite, 8, 8, SSD1327_WHITE);
  }
  if (confused) {
    display.drawBitmap(91, 116, confusionSprite, 8, 8, SSD1327_WHITE);
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
    if (tile == Wall || tile == DoorClosed)
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

bool isWalkable(int x, int y) {
  if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) return false;
  TileTypes tile = dungeonMap[y][x];
  // Walkable if floor or items/stairs/exits (adjust as needed)
  return (tile == Floor || tile == StartStairs || tile == Exit || tile == Freedom ||
          tile == Potion || tile == Map || tile == MushroomTile || tile == RingTile ||
          tile == ArmorTile || tile == ScrollTile || tile == DoorOpen || tile == RiddleStoneTile);
}

void unstuckEnemy(Enemy &enemy) {
  // Check if the enemy is stuck inside a wall
  int enemyGridX = round(enemy.x);
  int enemyGridY = round(enemy.y);
  
  if (!isWalkable(enemyGridX, enemyGridY)) {
    // Enemy is stuck in a wall, try to find a nearby walkable tile
    bool found = false;
    
    // Search in expanding radius (3x3, 5x5, 7x7 areas)
    for (int radius = 1; radius <= 3 && !found; radius++) {
      for (int dx = -radius; dx <= radius && !found; dx++) {
        for (int dy = -radius; dy <= radius; dy++) {
          // Only check tiles on the perimeter of the square
          if (abs(dx) != radius && abs(dy) != radius) continue;
          
          int checkX = enemyGridX + dx;
          int checkY = enemyGridY + dy;
          
          if (isWalkable(checkX, checkY)) {
            // Found a walkable tile, move enemy there
            enemy.x = (float)checkX;
            enemy.y = (float)checkY;
            found = true;
            break;
          }
        }
      }
    }
    
    // If still stuck, do a wider search in a larger area
    if (!found) {
      for (int dx = -5; dx <= 5 && !found; dx++) {
        for (int dy = -5; dy <= 5 && !found; dy++) {
          int checkX = enemyGridX + dx;
          int checkY = enemyGridY + dy;
          
          if (isWalkable(checkX, checkY)) {
            enemy.x = (float)checkX;
            enemy.y = (float)checkY;
            found = true;
            break;
          }
        }
      }
    }
  }
}

void drawWrappedText(int x, int y, int maxWidth, const char *text) {
  u8g2_for_adafruit_gfx.setCursor(x, y);

  int lineHeight = 10; // Adjust based on font size
  int cursorX = x;
  int cursorY = y;
  char currentLine[300] = "";
  char word[100] = "";
  size_t wordLen = 0;
  size_t currentLineLen = 0;
  size_t textLen = strlen(text);

  for (size_t i = 0; i < textLen; i++) {
    char c = text[i];

    if (c == ' ' || c == '\n') {
      // Check if adding this word would exceed maxWidth
      char testLine[400];
      snprintf(testLine, sizeof(testLine), "%s%s ", currentLine, word);
      int wordWidth = u8g2_for_adafruit_gfx.getUTF8Width(testLine);

      if (wordWidth > maxWidth) {
        // Print the current line before adding a new word
        u8g2_for_adafruit_gfx.setCursor(cursorX, cursorY);
        u8g2_for_adafruit_gfx.print(currentLine);
        cursorY += lineHeight;
        // Move the word to the new line
        snprintf(currentLine, sizeof(currentLine), "%s ", word);
        currentLineLen = strlen(currentLine);
      } else {
        // Append word to current line
        snprintf(currentLine + currentLineLen, sizeof(currentLine) - currentLineLen, "%s ", word);
        currentLineLen = strlen(currentLine);
      }

      word[0] = '\0';
      wordLen = 0;

      if (c == '\n') {  // Force a new line on explicit newline characters
        u8g2_for_adafruit_gfx.setCursor(cursorX, cursorY);
        u8g2_for_adafruit_gfx.print(currentLine);
        cursorY += lineHeight;
        currentLine[0] = '\0';
        currentLineLen = 0;
      }
    } else {
      // Append character to word
      if (wordLen < sizeof(word) - 1) {
        word[wordLen] = c;
        wordLen++;
        word[wordLen] = '\0';
      }
    }
  }

  // Print the remaining text
  if (currentLineLen > 0 || wordLen > 0) {
    char finalLine[400];
    snprintf(finalLine, sizeof(finalLine), "%s%s", currentLine, word);
    u8g2_for_adafruit_gfx.setCursor(cursorX, cursorY);
    u8g2_for_adafruit_gfx.print(finalLine);
  }
}

void updateScreenShake() {
  if (shakeDuration > 0) {
    offsetX += random(-shakeIntensity, shakeIntensity + 1);
    offsetY += random(-shakeIntensity, shakeIntensity + 1);
    shakeDuration--;
  }
}

void triggerScreenShake(int duration, int intensity) {
  shakeDuration = duration;
  shakeIntensity = intensity;
}

bool nearTile(TileTypes tile) {
  int rPx = round(playerX);
  int rPy = round(playerY);
  bool isNear = false;
  for (int dx = -1; dx <= 1 && !isNear; dx++) {
    for (int dy = -1; dy <= 1 && !isNear; dy++) {
      int cx = rPx + dx;
      int cy = rPy + dy;
      if (cx >= 0 && cx < mapWidth && cy >= 0 && cy < mapHeight && dungeonMap[cy][cx] == tile) {
        isNear = true;
      }
    }
  }
  return isNear;
}

void checkIfDeadFrom(const char *cause) {
  if (playerHP <= 0) {
    // If the riddle stone is equipped, trigger the riddle instead of death
    if (equippedRiddleStone) {
      currentUIState = UI_RIDDLE;
      equippedRiddleStone = false;
      for (int i = 0; i < inventorySize; i++) {
        if (strcmp(inventoryPages[2].items[i].itemResult, "Solve this riddle!") == 0) {
          removeItemFromInventory(2, i);
          break;
        }
      }
      return; // Exit early to prevent showing death screen
    }
    
    // No riddle stone, show death screen
    playRawSFX(10);
    snprintf(deathCause, sizeof(deathCause), "%s", cause);
    buttons.bPressedPrev = true;
    buttons.aPressedPrev = true;
    showDeathScreen = true;
  }
}

void trySaveGame() {
  saveData.armorValue = equippedArmorValue;
  saveData.attackDamage = playerAttackDamage;
  saveData.currentDungeon = dungeon;
  saveData.damsel = damsel[0];
  saveData.endlessMode = endlessMode;
  saveData.equippedArmor = equippedArmor;
  saveData.equippedRiddleStone = equippedRiddleStone;
  saveData.food = playerFood;
  saveData.hp = playerHP;
  for (int i = 0; i < numInventoryPages; i++) {
      saveData.savedInventory[i] = inventoryPages[i];
  }
  for (int i = 0; i < 30; i++) {
      saveData.savedEnemies[i] = enemies[i];
  }
  for (int y = 0; y < 64; y++) {
      for (int x = 0; x < 64; x++) {
          saveData.dungeonMap[y][x] = dungeonMap[y][x];
      }
  }

  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 20; x++) {
      saveData.scrollNames[y][x] = scrollNames[y][x];
    }
  }
  for (int i = 0; i < 31; i++) {
      saveData.itemList[i] = itemList[i];
  }
  saveData.hasMap = hasMap;
  saveData.playerNearClockEnemy = playerNearClockEnemy;
  saveData.knowsDamselName = knowsDamselName;
  saveData.damselSayThanksForRescue = damselSayThanksForRescue;
  saveData.damselGotTaken = damselGotTaken;
  saveData.levelOfDamselDeath = levelOfDamselDeath;

  saveData.kills = kills;
  saveData.playerX = playerX;
  saveData.playerY = playerY;
  saveData.strengthRingsNum = strengthRingsNumber;
  saveData.weaknessRingsNum = weaknessRingsNumber;
  saveData.swiftnessRingsNum = swiftnessRingsNumber;
  saveData.succubusFriend = succubusIsFriend;
  saveData.worldSeed = worldSeed;
  saveData.hungerRingsNumber = hungerRingsNumber;
  saveData.regenRingsNumber = regenRingsNumber;
  saveData.sicknessRingsNumber = sicknessRingsNumber;
  saveData.aggravateRingsNumber = aggravateRingsNumber;
  if (!saveGame(saveData)) {
    Serial.println("saveGame() failed");
  }
}

void tryLoadGame() {
  if (!loadGame(saveData)) {
    Serial.println("loadGame() failed — not applying saveData");
    return;
  }
  equippedArmorValue = saveData.armorValue;
  playerAttackDamage = saveData.attackDamage;
  dungeon = saveData.currentDungeon;
  damsel[0] = saveData.damsel;
  endlessMode = saveData.endlessMode;
  equippedArmor = saveData.equippedArmor;
  equippedRiddleStone = saveData.equippedRiddleStone;
  playerFood = saveData.food;
  playerHP = saveData.hp;
  for (int i = 0; i < numInventoryPages; i++) {
      inventoryPages[i] = saveData.savedInventory[i];
  }
  for (int i = 0; i < 30; i++) {
      enemies[i] = saveData.savedEnemies[i];
  }
  for (int y = 0; y < 64; y++) {
      for (int x = 0; x < 64; x++) {
          dungeonMap[y][x] = saveData.dungeonMap[y][x];
      }
  }

  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 20; x++) {
      scrollNames[y][x] = saveData.scrollNames[y][x];
    }
  }
  for (int i = 0; i < 31; i++) {
      itemList[i] = saveData.itemList[i];
  }
  hasMap = saveData.hasMap;
  playerNearClockEnemy = saveData.playerNearClockEnemy;
  knowsDamselName = saveData.knowsDamselName;
  damselSayThanksForRescue = saveData.damselSayThanksForRescue;
  damselGotTaken = saveData.damselGotTaken;
  levelOfDamselDeath = saveData.levelOfDamselDeath;

  kills = saveData.kills;
  playerX = saveData.playerX;
  playerY = saveData.playerY;
  strengthRingsNumber = saveData.strengthRingsNum;
  weaknessRingsNumber = saveData.weaknessRingsNum;
  swiftnessRingsNumber = saveData.swiftnessRingsNum;
  succubusIsFriend = saveData.succubusFriend;
  hungerRingsNumber = saveData.hungerRingsNumber;
  regenRingsNumber = saveData.regenRingsNumber;
  sicknessRingsNumber = saveData.sicknessRingsNumber;
  aggravateRingsNumber = saveData.aggravateRingsNumber;
  randomSeed(saveData.worldSeed);
}