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

  randomSeed(generateRandomSeed());

  generateRiddle();

  trainFemaleMarkov();
  
  // Assign a randomly generated name to the damsel
  damsel[0].name = generateFemaleName();

  display.begin();
  u8g2_for_adafruit_gfx.begin(display);
  u8g2_for_adafruit_gfx.setForegroundColor(15);
  display.setContrast(100);

  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_START_PIN, INPUT_PULLUP);

  // Initialize inventory
  for (int i = 0; i < inventorySize; i++) {
    for (int j = 0; j < numInventoryPages; j++) {
      inventoryPages[j].items[i] = { Null, PotionCategory, "Empty"};
      inventoryPages[j].itemCount = 0;
    }
  }

  // Randomize potion effects
  randomizePotionEffects();

  // Initialize projectiles
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
            renderGame();
            updateGame();
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

        case UI_PAUSE:
          handlePauseScreen();
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
  handleHunger();
  updateScrolling(viewportWidth, viewportHeight, scrollSpeed, offsetX, offsetY);
  updateDamsel();
  updateEnemies();
  updateProjectiles();
}

void renderGame() {
  display.clearDisplay();
  renderDungeon();
  renderDamsel();
  renderEnemies();
  renderProjectiles();
  renderPlayer();
  updateAnimations();
  renderUI();
  handleDialogue();
  display.display();
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

  display.clearDisplay();

  display.setCursor(7, 10);
  display.setTextSize(2);
  display.print("Game over!");
  display.setTextSize(1);
  display.setCursor(5, 30);
  display.print("Press [B] to restart");

  display.drawRect(8, 41, 110, 72, 15);

  if (page == 1) {
    display.setCursor(12, 44);
    display.print("Slain by:");
    display.setCursor(66, 44);
    display.print(deathCause.c_str());

    display.setCursor(12, 56);
    display.print("On dungeon:");
    display.setCursor(78, 56);
    display.print(Dngn);

    display.setCursor(12, 68);
    display.print("Dngn highscore:");
    display.setCursor(102, 68);
    display.print(DHighscore);

    display.setCursor(12, 80);
    display.print("Kills:");
    display.setCursor(48, 80);
    display.print(KLLS);

    display.setCursor(12, 92);
    display.print("Kll Highscore:");
    display.setCursor(96, 92);
    display.print(KHighscore);

    display.setCursor(22, 102);
    display.print("[A] next page");
  } else if (page == 2) {
    display.setCursor(12, 42);
    display.print("next page");
    display.setCursor(22, 102);
    display.print("[A] next page");
  }

  display.display();

  if (buttons.bPressed && !buttons.bPressedPrev) {
    playerHP = 100;
    playerFood = 100;
    dungeon = 1;
    levelOfDamselDeath = -4;
    generateDungeon();
    damsel[0].name = generateFemaleName();
    damsel[0].levelOfLove = 0;
    knowsDamselName = false;
    for (int i = 0; i < inventorySize; i++) {
      for (int j = 0; j < numInventoryPages; j++) {
        inventoryPages[j].items[i] = { Null, PotionCategory, "Empty"};
        inventoryPages[j].itemCount = 0;
      }
    }
    for (int i = 0; i < maxProjectiles; i++) {
      projectiles[i].active = false;
    }
    currentUIState = UI_NORMAL;
    showDialogue = false;
    speeding = false;
    currentSpeedMultiplier = 1;
    speedTimer = 1000;
    hasMap = false;
    resetPotionNames();
    randomizePotionEffects();
    spawnEnemies();
  }
}

bool leftDamsel = false;
void showStatusScreen() {
  static bool damselKidnapScreen = false; // Tracks if we are showing the kidnap screen

  display.clearDisplay();

  u8g2_for_adafruit_gfx.setFont(u8g2_font_profont10_mf);

  if (!damselKidnapScreen) {
    if (dungeon > levelOfDamselDeath + 3) {
      if (!damsel[0].dead && damsel[0].followingPlayer) {
        if (!carryingDamsel) {
          display.drawBitmap(0, -10, rescueDamselScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
        } else {
          display.drawBitmap(0, -10, carryDamselScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
        }
        u8g2_for_adafruit_gfx.setCursor(0, 125);
        u8g2_for_adafruit_gfx.print(F("You rescued the Damsel!"));
      } else {
        u8g2_for_adafruit_gfx.setCursor(0, 125);
        u8g2_for_adafruit_gfx.print(F("Error."));
      }
    } else if (dungeon == levelOfDamselDeath) {
      if (damsel[0].dead) {
        display.drawBitmap(0, -10, deadDamselScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
        u8g2_for_adafruit_gfx.setCursor(0, 105);
        if (!knowsDamselName) {
          u8g2_for_adafruit_gfx.print(F("You killed the Damsel!"));
        } else {
          String msg = "You killed " + damsel[0].name + "!";
          u8g2_for_adafruit_gfx.print(F(msg.c_str()));
        }
        u8g2_for_adafruit_gfx.setCursor(0, 115);
        u8g2_for_adafruit_gfx.print(F(damsel[0].levelOfLove >= 2 ? "She trusted you!" : "How could you!"));
        if (damsel[0].levelOfLove >= 5) {
          u8g2_for_adafruit_gfx.setCursor(0, 125);
          u8g2_for_adafruit_gfx.print(F("She loved you!"));
        }
      } else if (!damsel[0].dead && !damsel[0].followingPlayer) {
        display.drawBitmap(0, 0, leftDamselScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
        u8g2_for_adafruit_gfx.setCursor(0, 125);
        if (!knowsDamselName) {
          u8g2_for_adafruit_gfx.print(F("You left the Damsel!"));
        } else {
          String msg = "You left " + damsel[0].name + "!";
          u8g2_for_adafruit_gfx.print(F(msg.c_str()));
        }
        leftDamsel = true;
      }
    } else {
      display.drawBitmap(0, 0, aloneWizardScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      u8g2_for_adafruit_gfx.setCursor(0, 125);
      u8g2_for_adafruit_gfx.print(F("You progress. Alone."));
    }
  } else {
    display.drawBitmap(0, 0, capturedDamselScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
    u8g2_for_adafruit_gfx.setCursor(0, 10);
    u8g2_for_adafruit_gfx.print(F("The Damsel was captured!"));
  }

  display.display();

  // Handle button press logic
  if (buttons.bPressed && !buttons.bPressedPrev) { // Detect new button press
    if (damselKidnapScreen) {
      // Exit the kidnap screen
      damselKidnapScreen = false;
      statusScreen = false;
      damselSayThanksForRescue = true;
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
      damsel[0].levelOfLove += rescued && damselGotTaken ? 1 : 0;
      damsel[0].levelOfLove += rescued && carryingDamsel ? 1 : 0;
      damselGotTaken = rescued ? false : damselGotTaken;
      if (damsel[0].dead) {
        damsel[0].levelOfLove = 0;
        knowsDamselName = false;
        damsel[0].name = generateFemaleName();
      }
      if (leftDamsel) {
        damsel[0].levelOfLove = 0;
        knowsDamselName = false;
        damsel[0].name = generateFemaleName();
        leftDamsel = false;
      }

      if (rescued && randomChance == 3 && !carryingDamsel) {
        damselKidnapScreen = true; // Switch to the kidnap screen
        statusScreen = true;       // Keep status screen active
        damselGotTaken = true;
      } else if (rescued) {
        damsel[0].x = playerX;
        damsel[0].y = playerY - 1;
      }
    }
  }
}