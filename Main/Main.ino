#include <U8g2lib.h>
#include <EEPROM.h>
#include "Sprites.h"
#include "Dungeon.h"
#include "HelperFunctions.h"
#include "Entities.h"
#include "Item.h"

// OLED display pins
#define OLED_MOSI 11
#define OLED_CLK 13
#define OLED_DC 7
#define OLED_CS 10
#define OLED_RST 9
#define BUTTON_UP_PIN    2
#define BUTTON_DOWN_PIN  3
#define BUTTON_LEFT_PIN  4
#define BUTTON_RIGHT_PIN 5
#define BUTTON_B_PIN 1
#define BUTTON_A_PIN 0

struct ButtonStates {
  bool upPressed;
  bool upPressedPrev;
  bool downPressed;
  bool downPressedPrev;
  bool aPressed;
  bool aPressedPrev;
  bool bPressed;
  bool bPressedPrev;
};

ButtonStates buttons = {false};

// Viewport size (in tiles)
const int viewportWidth = 128 / tileSize;
const int viewportHeight = 128 / tileSize - 2;

// Map scrolling offset
float offsetX = 0;
float offsetY = 0;

// Smooth scrolling speed
const float scrollSpeed = 0.25f;

// Player position
float playerX;
float playerY;

// Player stats
int playerHP = 100;
int level = 1;
int kills = 0;
bool statusScreen = false;
unsigned int lvlHighscoreAddress = 0;
unsigned int killHighscoreAddress = 1;
String deathCause = "";
int levelOfDamselDeath = -4;

int playerDX;
int playerDY;

// SH1107 128x128 SPI Constructor
U8G2_SH1107_PIMORONI_128X128_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RST);

// Timing variables
unsigned long lastUpdateTime = 0;
const unsigned long frameDelay = 20; // Update every 100ms

enum UIState {
  UI_NORMAL,      // Normal gameplay
  UI_INVENTORY,   // Inventory screen
  UI_MINIMAP,     // Minimap screen
  UI_ITEM_ACTION, // Item action selection screen
  UI_ITEM_INFO    // Item info screen
};

// Add action selection tracking
int selectedActionIndex = 0; // 0 = Use, 1 = Drop, 2 = Info

UIState currentUIState = UI_NORMAL; // Current UI state

// Inventory variables
const int inventorySize = 8; // Number of inventory slots
GameItem inventory[inventorySize];
int selectedInventoryIndex = 0; // Currently selected inventory item

void setup() {
  Serial.begin(9600);
  u8g2.begin();
  u8g2.setBitmapMode(1);

  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  randomSeed(generateRandomSeed());

  for (int i = 0; i < inventorySize; i++) {
      inventory[i] = { RedPotion, "Empty", 0, 0, 0 };
  }
  randomizePotionEffects();

  for (int i = 0; i < maxProjectiles; i++) {
      projectiles[i].active = false;
  }

  // Generate a random dungeon
  generateDungeon(playerX, playerY, damsel[0], levelOfDamselDeath, level);
  spawnEnemies(playerX, playerY);
}

void loop() {
  updateButtonStates(); // Add this first
  
  unsigned long currentTime = millis();

  if (playerHP > 0) {
    if (!statusScreen) {
      handleUIStateTransitions();
      switch (currentUIState) {
        case UI_NORMAL:
          if (currentTime - lastUpdateTime >= frameDelay) {
            lastUpdateTime = currentTime;
            updateGame();
            renderGame();
          }
          break;

        case UI_INVENTORY:
          renderInventory();
          handleInventoryNavigation();
          handleInventoryItemUsage();
          break;

        case UI_MINIMAP:
          drawMinimap();
          break;

        case UI_ITEM_ACTION:
          handleItemActionMenu();
          renderInventory();
          break;

        case UI_ITEM_INFO:
          renderInventory();
          break;
      }
    } else {
      showStatusScreen();
    }
  } else {
    gameOver();
  }
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

void handleUIStateTransitions() {
  if (buttons.aPressed && !buttons.aPressedPrev) {
    switch (currentUIState) {
      case UI_NORMAL: 
        if (!statusScreen) currentUIState = UI_INVENTORY;
        break;
      case UI_INVENTORY: 
        currentUIState = UI_MINIMAP; 
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
    }
  }
}

void renderInventory() {
  u8g2.clearBuffer();

  // Draw inventory title
  if (currentUIState == UI_INVENTORY) {
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(10, 15, "Inventory");

    // Draw inventory items
    u8g2.setFont(u8g2_font_profont12_tr);
    for (int i = 0; i < inventorySize; i++) {
      int yPos = 30 + (i * 12);
      if (i == selectedInventoryIndex) {
        u8g2.drawStr(5, yPos, ">");
      }
      u8g2.drawStr(15, yPos, inventory[i].name.c_str());
    }
  } else if (currentUIState == UI_ITEM_INFO) {
    u8g2.setFont(u8g2_font_profont12_tr);
    // Assume the display width is 128 pixels and you want a margin of 3 pixels on the left.
    int x = 3;
    int y = 10;
    int maxWidth = 128 - x - 3; // adjust for margins as needed
    int lineHeight = 12;       // or choose an appropriate line height
    drawWrappedText(inventory[selectedInventoryIndex].description.c_str(), x, y, maxWidth, lineHeight);
  }

  // Draw action menu if active
  if (currentUIState == UI_ITEM_ACTION) {    
    // Background
    u8g2.drawFrame(50, 40, 60, 50);
    u8g2.drawBox(50, 40, 60, 12);
    
    // Title
    u8g2.setFont(u8g2_font_profont12_tr);
    u8g2.setDrawColor(0);
    u8g2.drawStr(55, 50, "Options:");
    u8g2.setDrawColor(1);
    
    // Actions
    u8g2.drawStr(55, 63, selectedActionIndex == 0 ? "> Use" : "  Use");
    u8g2.drawStr(55, 73, selectedActionIndex == 1 ? "> Drop" : "  Drop");
    u8g2.drawStr(55, 83, selectedActionIndex == 2 ? "> Info" : "  Info");
  }

  u8g2.sendBuffer();
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

// Add an item to the first empty slot
bool addToInventory(GameItem item) {
  for (int i = 0; i < inventorySize; i++) {
    if (strcmp(inventory[i].name.c_str(), "Empty") == 0) { // Check for empty slot
        inventory[i] = item;
        return true; // Successfully added
    }
  }
  return false; // Inventory full
}

void handleInventoryNavigation() {
  if (buttons.upPressed && !buttons.upPressedPrev && selectedInventoryIndex > 0) {
    selectedInventoryIndex--;
  }
  if (buttons.downPressed && !buttons.downPressedPrev && selectedInventoryIndex < inventorySize - 1) {
    selectedInventoryIndex++;
  }
}

void handleInventoryItemUsage() {
  if (buttons.bPressed && !buttons.bPressedPrev && currentUIState == UI_INVENTORY) {
    GameItem &selectedItem = inventory[selectedInventoryIndex];
    
    if (strcmp(selectedItem.name.c_str(), "Empty") != 0) {
      currentUIState = UI_ITEM_ACTION;
      selectedActionIndex = 0;
    }
  }
}

void handleItemActionMenu() {
  // Navigation
  if (buttons.upPressed && !buttons.upPressedPrev) {
    selectedActionIndex--;
  }
  if (buttons.downPressed && !buttons.downPressedPrev) {
    selectedActionIndex++;
  }

  selectedActionIndex = selectedActionIndex == 3 ? 0 : selectedActionIndex == -1 ? 0 : selectedActionIndex;

  // Cancel with A
  if (buttons.aPressed && !buttons.aPressedPrev) {
    currentUIState = UI_INVENTORY;
  }

  // Confirm with B
  if (buttons.bPressed && !buttons.bPressedPrev) {
    GameItem &selectedItem = inventory[selectedInventoryIndex];
    
    if (selectedActionIndex == 0) { // Use
      if (selectedItem.item >= RedPotion && selectedItem.item <= YellowPotion) {
        playerHP += selectedItem.healthRecoverAmount;
        if (playerHP <= 0) deathCause = "poison";
        
        if (selectedItem.AOEsize > 0) {
          applyAOEEffect(playerX, playerY, selectedItem.AOEsize, selectedItem.AOEdamage);
        }
        
        for (int i = 0; i < inventorySize; i++) {
          if (inventory[i].item == selectedItem.item) {
            updatePotionName(inventory[i]);
          }
        }
      }
      inventory[selectedInventoryIndex] = { RedPotion, "Empty", 0, 0, 0 };
    }
    else if (selectedActionIndex == 1) { // Drop
      inventory[selectedInventoryIndex] = { RedPotion, "Empty", 0, 0, 0 };
    } else { // Info
      currentUIState = UI_ITEM_INFO;
    }
    
    if (currentUIState != UI_ITEM_INFO) {
      currentUIState = UI_INVENTORY;
    }
  }
}

void applyAOEEffect(float centerX, float centerY, int aoeRadius, int aoeDamage) {
  // Loop through all enemies
  for (int i = 0; i < maxEnemies; i++) {
    // Only consider enemies that are still alive
    if (enemies[i].hp > 0) {
      // Use the same rounding as your collision functions:
      int dx = round(centerX) - round(enemies[i].x);
      int dy = round(centerY) - round(enemies[i].y);
      // Compare squared distance to avoid computing square roots
      if (dx * dx + dy * dy <= aoeRadius * aoeRadius) {
        enemies[i].hp -= aoeDamage;
        if (enemies[i].hp <= 0) {
          kills += 1;
        }
      }
    }
  }
}

void updateGame() {
  handleInput();
  updateScrolling();
  updateDamsel(playerDX, playerDY, playerX, playerY);
  updateEnemies(playerHP, playerX, playerY, deathCause);
  updateProjectiles(kills, levelOfDamselDeath, level);
}

void renderGame() {
  if (currentUIState == UI_NORMAL) {
    u8g2.clearBuffer();
    renderDungeon();
    renderDamsel();
    renderEnemies();
    renderProjectiles();
    renderPlayer();
    updateAnimations();
    renderUI();
    u8g2.sendBuffer();
  }
}

void drawMinimap() {
    u8g2.clearBuffer();
    int mapScale = 2;
    
    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            int tile = dungeonMap[y][x];
            int drawX = x * mapScale;
            int drawY = y * mapScale;
            
            if (tile == 1) continue;
            if (tile == 2) u8g2.drawBox(drawX, drawY, mapScale, mapScale);
            if (tile == 3) u8g2.drawCircle(drawX, drawY, mapScale/2);
        }
    }
    
    int playerMinimapX = (playerX) * mapScale;
    int playerMinimapY = (playerY) * mapScale;
    u8g2.drawCircle(playerMinimapX, playerMinimapY, 1);
    
    u8g2.sendBuffer();
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

void updateScrolling() {
  // Target offsets based on player's position
  float targetOffsetX = playerX - (viewportWidth / 2.0f) + 0.5f;
  float targetOffsetY = playerY - (viewportHeight / 2.0f) + 0.5f;

  // Clamp target offsets to map boundaries
  targetOffsetX = constrain(targetOffsetX, 0, mapWidth - viewportWidth);
  targetOffsetY = constrain(targetOffsetY, 0, mapHeight - viewportHeight);

  // Smoothly move the offset towards the target
  offsetX += (targetOffsetX - offsetX) * scrollSpeed;
  offsetY += (targetOffsetY - offsetY) * scrollSpeed;
}

// Render the visible portion of the dungeon
void renderDungeon() {
  for (int y = 1; y < viewportHeight + 1; y++) { // +1 to handle partial tiles at edges
    for (int x = 1; x < viewportWidth + 1; x++) {
      float mapX = x + offsetX;
      float mapY = y + offsetY;

      if (mapX >= 0 && mapX < mapWidth && mapY >= 0 && mapY < mapHeight) {
        // Calculate screen position based on fractional offsets
        float screenX = (x - (offsetX - (int)offsetX)) * tileSize;
        float screenY = (y - (offsetY - (int)offsetY)) * tileSize;

        drawTile((int)mapX, (int)mapY, screenX, screenY);
      }
    }
  }
}

// Draw a tile based on its type
void drawTile(int mapX, int mapY, float screenX, float screenY) {
  int tileType = dungeonMap[mapY][mapX];

  switch (tileType) {
    case 0: // Start stairs
      u8g2.drawXBMP(screenX, screenY, tileSize, tileSize, stairsSprite);
      break;
    case 1: // Floor
      //u8g2.drawFrame(screenX, screenY, tileSize, tileSize);
      break;
    case 2: // Wall
      //u8g2.drawBox(screenX, screenY, tileSize, tileSize);
      u8g2.drawXBMP(screenX, screenY, tileSize, tileSize, wallSprite);
      break;
    case 3: // Bars
      u8g2.drawXBMP(screenX, screenY, tileSize, tileSize, barsSprite);
      break;
    case 4: // Exit
      u8g2.drawXBMP(screenX, screenY, tileSize, tileSize, stairsSprite);
      break;
    case 5: // Exit
      u8g2.drawXBMP(screenX, screenY, tileSize, tileSize, potionSprite);
      break;
  }
}

// Render the player
void renderPlayer() {
  float screenX = (playerX - offsetX) * tileSize;
  float screenY = (playerY - offsetY) * tileSize;

  // Ensure the player is within the viewport
  if (screenX >= 0 && screenX < 128 && screenY >= 0 && screenY < 128) {
    //u8g2.drawDisc(screenX + tileSize / 2, screenY + tileSize / 2, tileSize / 3, U8G2_DRAW_ALL);
    u8g2.drawXBMP((screenX + tileSize / 2) - tileSize/2, (screenY + tileSize / 2) - tileSize/2, tileSize, tileSize, playerSprite);
  }
}

void renderEnemies() {
  for (int i = 0; i < maxEnemies; i++) {
    if (enemies[i].hp > 0) {
      float screenX = (enemies[i].x - offsetX) * tileSize;
      float screenY = (enemies[i].y - offsetY) * tileSize;
      if (screenX >= 0 && screenY >= 0 && screenX < 128 && screenY < 128) {
        u8g2.drawXBMP(screenX, screenY, 8, 8, blobSprite);
      }
    }
  }
}

void renderDamsel() {
  float screenX = (damsel[0].x - offsetX) * tileSize;
  float screenY = (damsel[0].y - offsetY) * tileSize;
  if (screenX >= 0 && screenY >= 0 && screenX < 128 && screenY < 128) {
    u8g2.drawXBMP(screenX, screenY, 8, 8, damselSprite);
  }
}

void shootProjectile(float xDir, float yDir) {

  for (int i = 0; i < maxProjectiles; i++) {
      if (!projectiles[i].active) {
          projectiles[i].x = playerX;
          projectiles[i].y = playerY;
          projectiles[i].dx = xDir;  // Set direction based on player's facing direction
          projectiles[i].dy = yDir;
          projectiles[i].damage = 10;
          projectiles[i].speed = 0.5;
          projectiles[i].active = true;
          break;
      }
  }
}

void renderProjectiles() {
    for (int i = 0; i < maxProjectiles; i++) {
        if (projectiles[i].active) {
          float screenX = (projectiles[i].x - offsetX) * tileSize + tileSize/2;
          float screenY = (projectiles[i].y - offsetY) * tileSize + tileSize/2;
          u8g2.drawDisc(screenX, screenY, 1);
        }
    }
}

// Render the UI
void renderUI() { 
  char HP[4];
  char Lvl[7];
  snprintf(HP, sizeof(HP), "%d", playerHP); // Convert playerHP to a string
  snprintf(Lvl, sizeof(Lvl), "%d", level);
  
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(5, 123, "HP:");
  u8g2.drawStr(20, 123, HP);
  u8g2.drawStr(40, 123, "LVL:");
  u8g2.drawStr(60, 123, Lvl);
  u8g2.drawFrame(0, 113, 128, 15);
}

int shootDelay = 0;
bool reloading;
void handleInput() {
  float newX = playerX;
  float newY = playerY;

  // Read button states (inverted because of pull-up resistors)
  bool upPressed = !digitalRead(BUTTON_UP_PIN);
  bool downPressed = !digitalRead(BUTTON_DOWN_PIN);
  bool leftPressed = !digitalRead(BUTTON_LEFT_PIN);
  bool rightPressed = !digitalRead(BUTTON_RIGHT_PIN);
  bool bPressed = !digitalRead(BUTTON_B_PIN);
  //bool aPressed = !digitalRead(BUTTON_A_PIN);

  if (upPressed && !leftPressed && !rightPressed) {
    playerDY = -1;
    playerDX = 0;
    newY -= 0.1; // Move up
  } else if (downPressed && !leftPressed && !rightPressed) {
    playerDY = 1;
    playerDX = 0;
    newY += 0.1; // Move down
  } else if (leftPressed && !upPressed && !downPressed) {
    playerDX = -1;
    playerDY = 0;
    playerSprite = playerSpriteLeft;
    newX -= 0.1; // Move left
  } else if (rightPressed && !upPressed && !downPressed) {
    playerDX = 1;
    playerDY = 0;
    playerSprite = playerSpriteRight;
    newX += 0.1; // Move right
  } else if (upPressed && leftPressed) {
    playerDY = -1;
    playerDX = -1;
    playerSprite = playerSpriteLeft;
    newY -= 0.1; // Move up & left
    newX -= 0.1; // Move up & left
  } else if (upPressed && rightPressed) {
    playerDY = -1;
    playerDX = 1;
    playerSprite = playerSpriteRight;
    newY -= 0.1; // Move up & right
    newX += 0.1; // Move up & left
  } else if (downPressed && leftPressed) {
    playerDX = -1;
    playerDY = 1;
    playerSprite = playerSpriteLeft;
    newX -= 0.1; // Move left & down
    newY += 0.1; // Move up & left
  } else if (downPressed && rightPressed) {
    playerDX = 1;
    playerDY = 1;
    playerSprite = playerSpriteRight;
    newX += 0.1; // Move right & down
    newY += 0.1; // Move up & left
  }

  if (bPressed && !reloading) {
    shootProjectile(playerDX, playerDY); // Shoot in current direction
    reloading = true;
  }

  if (reloading) {
    shootDelay++;
    if (shootDelay >= 10) {
      reloading = false;
      shootDelay = 0;
    }
  }

  if (Serial.available() > 0) {// for debug purposes
    char input = Serial.read();
    if (input == '7') {
      setTile((int)playerX, (int)playerY, 4);
    } else if (input == '8') {
      moveDamselToPos(playerX, playerY);
      if (!damsel[0].active) {
        Serial.println("The damsel is not active.");
      }
    }
  }

  int rNewX = round(newX);
  int rNewY = round(newY);

  // Check collision with walls
  if (dungeonMap[rNewY][rNewX] == 1 || dungeonMap[rNewY][rNewX] == 4 || dungeonMap[rNewY][rNewX] == 0) {
    playerX = newX;
    playerY = newY;

    // Update viewport offset if needed
    if (playerX - offsetX < 2 && offsetX > 0) offsetX -= scrollSpeed;
    if (playerX - offsetX > viewportWidth - 3 && offsetX < mapWidth - viewportWidth) offsetX += scrollSpeed;
    if (playerY - offsetY < 2 && offsetY > 0) offsetY -= scrollSpeed;
    if (playerY - offsetY > viewportHeight - 3 && offsetY < mapHeight - viewportHeight) offsetY += scrollSpeed;
  } else if (dungeonMap[rNewY][rNewX] == 5) {
    dungeonMap[rNewY][rNewX] = 1;
    addToInventory(getItem(getRandomPotion(random(6))));
  }

  int rPx = round(playerX);
  int rPy = round(playerY);

  // Check if the player reached the exit
  if (dungeonMap[rPy][rPx] == 4) {
    Serial.println("You reached the exit!");
    if (!damsel[0].dead && !damsel[0].followingPlayer && damsel[0].active) {
      levelOfDamselDeath = level;
      damsel[0].active = false;
    }
    statusScreen = true;
  }
}

bool pressed;
int page = 1;
int pageDelay = 0;
bool bWasPressedOnDeath = false;  // NEW: Tracks if B was pressed when player died

void gameOver() {
  bool bPressed = !digitalRead(BUTTON_B_PIN);
  bool aPressed = !digitalRead(BUTTON_A_PIN);

  if (aPressed && !pressed) {
    page++;
    if (page == 3) {
      page = 1;
    }
    pressed = true;
  }

  if (!aPressed) {
    pageDelay = 0;
    pressed = false;
  }

  if (pressed) {
    pageDelay++;
    if (pageDelay >= 50) {
      pageDelay = 0;
      pressed = false;
    }
  }

  // Check if B was held on death, and only allow restart if it has been released
  if (!bPressed) {
    bWasPressedOnDeath = false;  // Player has released B, so allow restart
  }

  char Lvl[7];
  snprintf(Lvl, sizeof(Lvl), "%d", level);
  char KLLS[7];
  snprintf(KLLS, sizeof(KLLS), "%d", kills);

  int lvlHighscore = EEPROM.read(lvlHighscoreAddress);
  if (level > lvlHighscore) {
    EEPROM.update(lvlHighscoreAddress, level);
  }

  int kllHighscore = EEPROM.read(killHighscoreAddress);
  if (kills > kllHighscore) {
    EEPROM.update(killHighscoreAddress, kills);
  }

  char LHighscore[7];
  snprintf(LHighscore, sizeof(LHighscore), "%d", lvlHighscore);
  char KHighscore[7];
  snprintf(KHighscore, sizeof(KHighscore), "%d", kllHighscore);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(11, 30, "Game over!");

  u8g2.drawFrame(11, 42, 108, 80);

  u8g2.setFont(u8g2_font_profont12_tr);
  if (page == 1) {
    u8g2.drawStr(15, 54, "Slain by:");
    u8g2.drawStr(70, 54, deathCause.c_str());

    u8g2.drawStr(15, 66, "On level:");
    u8g2.drawStr(70, 66, Lvl);

    u8g2.drawStr(15, 78, "Lvl highscore:");
    u8g2.drawStr(100, 78, LHighscore);

    u8g2.drawStr(15, 90, "Kills:");
    u8g2.drawStr(52, 90, KLLS);

    u8g2.drawStr(15, 102, "Kll Highscore:");
    u8g2.drawStr(100, 102, KHighscore);

    u8g2.drawStr(15, 114, "[A] next page");
  } else if (page == 2) {
    u8g2.drawStr(15, 54, "next page");
    u8g2.drawStr(15, 114, "[A] next page");
  }

  u8g2.sendBuffer();

  // Only allow restart if B was **not** pressed when the player died and is now being pressed
  if (bPressed && !bWasPressedOnDeath) {
    bWasPressedOnDeath = true;  // Mark that B was pressed to prevent instant restart

    playerHP = 100;
    level = 1;
    levelOfDamselDeath = -4;
    generateDungeon(playerX, playerY, damsel[0], levelOfDamselDeath, level);
    for (int i = 0; i < inventorySize; i++) {
      inventory[i] = { RedPotion, "Empty", 0, 0, 0 };
    }
    for (int i = 0; i < maxProjectiles; i++) {
      projectiles[i].active = false;
    }
    currentUIState = UI_NORMAL;
    resetPotionNames();
    randomizePotionEffects();
    spawnEnemies(playerX, playerY);
  }
}

void showStatusScreen() {
  static bool damselKidnapScreen = false; // Tracks if we are showing the kidnap screen

  u8g2.clearBuffer();

  if (!damselKidnapScreen) {
    if (level > levelOfDamselDeath + 3) {
      if (!damsel[0].dead && damsel[0].followingPlayer) {
        u8g2.drawXBMP(0, -10, 128, 128, rescueDamselScreen);
        u8g2.drawStr(0, 125, "You rescued the Damsel!");
      } else {
        u8g2.drawStr(0, 125, "Error.");
      }
    } else if (level == levelOfDamselDeath) {
      if (damsel[0].dead) {
        u8g2.drawXBMP(0, -10, 128, 128, deadDamselScreen);
        u8g2.drawStr(0, 105, "You killed the Damsel!");
        u8g2.drawStr(0, 115, "How could you!");//                                                   change to "she trusted you!" and add "she loved you!" when the level of love (implement later) is high enough
      } else if (!damsel[0].dead && !damsel[0].followingPlayer) {
        u8g2.drawXBMP(0, 0, 128, 128, leftDamselScreen);
        u8g2.drawStr(0, 125, "You left the Damsel!");
      }
    } else {
      u8g2.drawXBMP(0,0, 128, 128, aloneWizardScreen);
      u8g2.drawStr(0, 125, "You progress. Alone.");
    }
  } else {
    u8g2.drawXBMP(0,0, 128, 128, capturedDamselScreen);
    u8g2.drawStr(0, 10, "The Damsel was captured!");
  }

  u8g2.sendBuffer();

  // Handle button press logic
  if (buttons.bPressed && !buttons.bPressedPrev) { // Detect new button press
    if (damselKidnapScreen) {
      // Exit the kidnap screen
      damselKidnapScreen = false;
      statusScreen = false;
    } else if (statusScreen) {
      bool rescued = damsel[0].active && !damsel[0].dead && damsel[0].followingPlayer;

      level += 1;
      playerDX = 0;
      playerDY = 1;
      statusScreen = false;
      generateDungeon(playerX, playerY, damsel[0], levelOfDamselDeath, level); // Generate a new dungeon
      for (int i = 0; i < maxProjectiles; i++) {
        projectiles[i].active = false;
      }
      spawnEnemies(playerX, playerY);

      int randomChance = random(1, 5);

      if (rescued && randomChance == 3) {
        damselKidnapScreen = true; // Switch to the kidnap screen
        statusScreen = true;       // Keep status screen active
      } else if (rescued) {
        damsel[0].x = playerX;
        damsel[0].y = playerY - 1;
      }
    }
  }
}
