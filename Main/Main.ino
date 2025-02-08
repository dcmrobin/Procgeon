#include <EEPROM.h>
#include "Sprites.h"
#include "Dungeon.h"
#include "HelperFunctions.h"
#include "Entities.h"
#include "Item.h"
#include "Inventory.h"

// Smooth scrolling speed
const float scrollSpeed = 0.25f;

// Player position
float playerX;
float playerY;

bool itemResultScreenActive = false;

// Player stats
int playerHP = 100;
int playerMaxHP = 100;
int level = 1;
int kills = 0;
unsigned int lvlHighscoreAddress = 0;
unsigned int killHighscoreAddress = 1;
String deathCause = "";
int levelOfDamselDeath = -4;
bool speeding;
int speedTimer = 1000;
bool hasMap;

int playerDX;
int playerDY;

// Timing variables
unsigned long lastUpdateTime = 0;
const unsigned long frameDelay = 20; // Update every 100ms

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
      inventory[i] = { Null, "Empty", 0, 0, 0 };
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
  updateButtonStates();
  
  unsigned long currentTime = millis();

  if (playerHP > 0) {
    if (!statusScreen) {
      handleUIStateTransitions(hasMap);
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
          drawMinimap(playerX, playerY);
          break;

        case UI_ITEM_ACTION:
          handleItemActionMenu(playerHP, playerMaxHP, playerX, playerY, deathCause, speeding, kills, speedTimer);
          renderInventory();
          break;

        case UI_ITEM_INFO:
          renderInventory();
          break;

        case UI_ITEM_RESULT:
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

void updateGame() {
  handleInput();
  updateScrolling(playerX, playerY, viewportWidth, viewportHeight, scrollSpeed, offsetX, offsetY);
  updateDamsel(playerDX, playerDY, playerX, playerY);
  updateEnemies(playerHP, playerX, playerY, deathCause);
  updateProjectiles(kills, levelOfDamselDeath, level);
}

void renderGame() {
  u8g2.clearBuffer();
  renderDungeon();
  renderDamsel(playerX, playerY);
  renderEnemies(playerX, playerY);
  renderProjectiles();
  renderPlayer();
  updateAnimations();
  renderUI(playerHP, level, hasMap);
  u8g2.sendBuffer();
}

// Render the player
void renderPlayer() {
  float screenX = (playerX - offsetX) * tileSize;
  float screenY = (playerY - offsetY) * tileSize;

  // Ensure the player is within the viewport
  if (screenX >= 0 && screenX < SCREEN_WIDTH && screenY >= 0 && screenY < SCREEN_HEIGHT) {
    u8g2.drawXBMP((screenX + tileSize / 2) - tileSize/2, (screenY + tileSize / 2) - tileSize/2, tileSize, tileSize, playerSprite);
  }
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
  float speed = speeding ? 0.2 : 0.1;

  if (speeding) {
    speedTimer--;
    if (speedTimer <= 0) {
      speedTimer = 1000;
      speeding = false;
    }
  }

  if (upPressed && !leftPressed && !rightPressed) {
    playerDY = -1;
    playerDX = 0;
    newY -= speed; // Move up
  } else if (downPressed && !leftPressed && !rightPressed) {
    playerDY = 1;
    playerDX = 0;
    newY += speed; // Move down
  } else if (leftPressed && !upPressed && !downPressed) {
    playerDX = -1;
    playerDY = 0;
    playerSprite = playerSpriteLeft;
    newX -= speed; // Move left
  } else if (rightPressed && !upPressed && !downPressed) {
    playerDX = 1;
    playerDY = 0;
    playerSprite = playerSpriteRight;
    newX += speed; // Move right
  } else if (upPressed && leftPressed) {
    playerDY = -1;
    playerDX = -1;
    playerSprite = playerSpriteLeft;
    newY -= speed; // Move up & left
    newX -= speed; // Move up & left
  } else if (upPressed && rightPressed) {
    playerDY = -1;
    playerDX = 1;
    playerSprite = playerSpriteRight;
    newY -= speed; // Move up & right
    newX += speed; // Move up & left
  } else if (downPressed && leftPressed) {
    playerDX = -1;
    playerDY = 1;
    playerSprite = playerSpriteLeft;
    newX -= speed; // Move left & down
    newY += speed; // Move up & left
  } else if (downPressed && rightPressed) {
    playerDX = 1;
    playerDY = 1;
    playerSprite = playerSpriteRight;
    newX += speed; // Move right & down
    newY += speed; // Move up & left
  }

  if (bPressed && !reloading) {
    shootProjectile(playerDX, playerDY, playerX, playerY); // Shoot in current direction
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
    if (addToInventory(getItem(getRandomPotion(random(7))))) {
      dungeonMap[rNewY][rNewX] = 1;
    }
  } else if (dungeonMap[rNewY][rNewX] == 6) {
    hasMap = true;
    dungeonMap[rNewY][rNewX] = 1;
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

int page = 1;

void gameOver() {
  if (buttons.aPressed && !buttons.aPressedPrev) {
    page++;
    if (page == 3) {
      page = 1;
    }
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

  if (buttons.bPressed && !buttons.bPressedPrev) {
    playerHP = 100;
    level = 1;
    levelOfDamselDeath = -4;
    generateDungeon(playerX, playerY, damsel[0], levelOfDamselDeath, level);
    for (int i = 0; i < inventorySize; i++) {
      inventory[i] = { Null, "Empty", 0, 0, 0 };
    }
    for (int i = 0; i < maxProjectiles; i++) {
      projectiles[i].active = false;
    }
    currentUIState = UI_NORMAL;
    speeding = false;
    speedTimer = 1000;
    hasMap = false;
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
        u8g2.drawXBMP(0, -10, SCREEN_WIDTH, SCREEN_HEIGHT, rescueDamselScreen);
        u8g2.drawStr(0, 125, "You rescued the Damsel!");
      } else {
        u8g2.drawStr(0, 125, "Error.");
      }
    } else if (level == levelOfDamselDeath) {
      if (damsel[0].dead) {
        u8g2.drawXBMP(0, -10, SCREEN_WIDTH, SCREEN_HEIGHT, deadDamselScreen);
        u8g2.drawStr(0, 105, "You killed the Damsel!");
        u8g2.drawStr(0, 115, damsel[0].levelOfLove >= 2 ? "She trusted you!" : "How could you!");
        if (damsel[0].levelOfLove >= 5) {u8g2.drawStr(0, 125, "She loved you!");}
      } else if (!damsel[0].dead && !damsel[0].followingPlayer) {
        u8g2.drawXBMP(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, leftDamselScreen);
        u8g2.drawStr(0, 125, "You left the Damsel!");
        damsel[0].levelOfLove = 0;
      }
    } else {
      u8g2.drawXBMP(0,0, SCREEN_WIDTH, SCREEN_HEIGHT, aloneWizardScreen);
      u8g2.drawStr(0, 125, "You progress. Alone.");
    }
  } else {
    u8g2.drawXBMP(0,0, SCREEN_WIDTH, SCREEN_HEIGHT, capturedDamselScreen);
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

      hasMap = false;

      int randomChance = random(1, 5);

      damsel[0].levelOfLove += rescued ? 1 : 0;
      if (damsel[0].dead) {
        damsel[0].levelOfLove = 0;
      }

      char Love[7];
      snprintf(Love, sizeof(Love), "%d", damsel[0].levelOfLove);

      if (Serial.available() > 0) {
        Serial.println(Love);
      }

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
