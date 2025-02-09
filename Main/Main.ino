#include <EEPROM.h>
#include "Sprites.h"
#include "Dungeon.h"
#include "HelperFunctions.h"
#include "Entities.h"
#include "Item.h"
#include "Inventory.h"
#include "Player.h"

bool itemResultScreenActive = false;

unsigned int dngnHighscoreAddress = 0;
unsigned int killHighscoreAddress = 1;

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
  generateDungeon();
  spawnEnemies();
}

void loop() {
  updateButtonStates();
  
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
  updateScrolling(viewportWidth, viewportHeight, scrollSpeed, offsetX, offsetY);
  updateDamsel();
  updateEnemies();
  updateProjectiles();
}

void renderGame() {
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

int page = 1;
void gameOver() {
  if (buttons.aPressed && !buttons.aPressedPrev) {
    page++;
    if (page == 3) {
      page = 1;
    }
  }

  char Dngn[7];
  snprintf(Dngn, sizeof(Dngn), "%d", dungeon);
  char KLLS[7];
  snprintf(KLLS, sizeof(KLLS), "%d", kills);

  int dngnHighscore = EEPROM.read(dngnHighscoreAddress);
  if (dungeon > dngnHighscore) {
    EEPROM.update(dngnHighscoreAddress, dungeon);
  }

  int kllHighscore = EEPROM.read(killHighscoreAddress);
  if (kills > kllHighscore) {
    EEPROM.update(killHighscoreAddress, kills);
  }

  char DHighscore[7];
  snprintf(DHighscore, sizeof(DHighscore), "%d", dngnHighscore);
  char KHighscore[7];
  snprintf(KHighscore, sizeof(KHighscore), "%d", kllHighscore);

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(11, 30, "Game over!");

  u8g2.drawFrame(10, 42, 110, 80);

  u8g2.setFont(u8g2_font_profont12_tr);
  if (page == 1) {
    u8g2.drawStr(15, 54, "Slain by:");
    u8g2.drawStr(70, 54, deathCause.c_str());

    u8g2.drawStr(15, 66, "On dungeon:");
    u8g2.drawStr(82, 66, Dngn);

    u8g2.drawStr(15, 78, "Dngn highscore:");
    u8g2.drawStr(106, 78, DHighscore);

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
    dungeon = 1;
    levelOfDamselDeath = -4;
    generateDungeon();
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
    spawnEnemies();
  }
}

void showStatusScreen() {
  static bool damselKidnapScreen = false; // Tracks if we are showing the kidnap screen

  u8g2.clearBuffer();

  if (!damselKidnapScreen) {
    if (dungeon > levelOfDamselDeath + 3) {
      if (!damsel[0].dead && damsel[0].followingPlayer) {
        u8g2.drawXBMP(0, -10, SCREEN_WIDTH, SCREEN_HEIGHT, rescueDamselScreen);
        u8g2.drawStr(0, 125, "You rescued the Damsel!");
      } else {
        u8g2.drawStr(0, 125, "Error.");
      }
    } else if (dungeon == levelOfDamselDeath) {
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

      dungeon += 1;
      playerDX = 0;
      playerDY = 1;
      statusScreen = false;
      generateDungeon(); // Generate a new dungeon
      for (int i = 0; i < maxProjectiles; i++) {
        projectiles[i].active = false;
      }
      spawnEnemies();

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
