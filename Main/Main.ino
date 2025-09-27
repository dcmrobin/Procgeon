#include <EEPROM.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "Sprites.h"
#include "Dungeon.h"
#include "HelperFunctions.h"
#include "Entities.h"
#include "Item.h"
#include "Inventory.h"
#include "Player.h"
#include "GameAudio.h"


bool itemResultScreenActive = false;

unsigned int dngnHighscoreAddress = 0;
unsigned int killHighscoreAddress = 1;

// Timing variables
unsigned long lastUpdateTime = 0;
const unsigned long frameDelay = 20; // Update every 100ms

// SD card chip select pin for Teensy Audio Board
const int SD_CS = BUILTIN_SDCARD;  // For Teensy 4.1 with built-in SD slot

void resetGame() {
  // Reset player stats
  playerHP = 100;
  playerFood = 100;
  dungeon = 1;
  levelOfDamselDeath = -4;
  kills = 0;
  
  // Reset damsel
  damsel[0].name = generateFemaleName();
  damsel[0].levelOfLove = 0;
  knowsDamselName = false;
  
  // Reset inventory
  for (int i = 0; i < inventorySize; i++) {
    for (int j = 0; j < numInventoryPages; j++) {
      inventoryPages[j].items[i] = { Null, PotionCategory, "Empty"};
      inventoryPages[j].itemCount = 0;
    }
  }
  
  // Give player a cloak and equip it
  GameItem cloak = getItem(Cloak);
  cloak.isEquipped = true;
  addToInventory(cloak, false);
  equippedArmorValue = cloak.armorValue;
  equippedArmor = cloak;
  
  // Reset projectiles
  for (int i = 0; i < maxProjectiles; i++) {
    projectiles[i].active = false;
  }
  
  // Reset UI and game state
  currentUIState = UI_NORMAL;
  speeding = false;
  currentSpeedMultiplier = 1;
  speedTimer = 500;
  hasMap = false;
  equippedRiddleStone = false;
  starving = false;
  seeAll = false;
  confused = false;
  ridiculed = false;
  glamoured = false;
  blinded = false;
  showDialogue = false;
  paralyzed = false;
  
  // Reset damsel state
  damselWasFollowing = false;
  damselWaitUpTimer = 0;
  damselSaidWaitUp = false;
  
  // Reset potion effects
  resetPotionNames();
  randomizePotionEffects();
  randomizeScrollEffects();
  randomizeRingEffects();

  // --- Explicitly reset all ring and speed effect flags ---
  ringOfSwiftnessActive = false;
  ringOfStrengthActive = false;
  ringOfWeaknessActive = false;
  ringOfHungerActive = false;
  ringOfRegenActive = false;
  lastPotionSpeedModifier = 0;
  playerAttackDamage = 10;

  // Generate new dungeon and spawn enemies
  generateDungeon();
  spawnEnemies();
}

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 4000); // Wait for Serial Monitor
  if (CrashReport) {
    Serial.print(CrashReport);
  }

  initAudio();

  // Initialize SD card
  if (!SD.begin(SD_CS)) {
    Serial.println("SD initialization failed!");
    while (1);  // Stop execution
  }
  Serial.println("SD initialization done.");

  // Play a sound effect from memory
  if (!loadSFXtoRAM()) {
    Serial.println("Failed to load SFX to RAM");
  } else {
    Serial.println("SFX loaded successfully");
  }

  Serial.println("type 8: teleport damsel to player if damsel is available");
  Serial.println("type 7: make tile player is on into the exit");
  Serial.println("type 6: add potion to inventory");
  Serial.println("type 5: make tile player is on into a riddlestone");
  Serial.println("type 4: make tile player is on into a mushroom");
  Serial.println("type 3: make tile player is on into an armor");
  Serial.println("type 2: make tile player is on into a scroll");
  Serial.println("type 1: make tile player is on into a ring");

  // Play a WAV file
  /*if (playWav1.play("bossfight.wav")) {
    Serial.println("bossfight.wav played successfully");
  } else {
    Serial.println("bossfight.wav failed to play");
  }

  if (!playWav1.isPlaying()) {
      Serial.println("bossfight.wav is not playing");
  } else {
      Serial.println("bossfight.wav is playing");
  }*/

  //if (SD.exists("bossfight.wav")) {
  //  Serial.println("bossfight.wav does exist");
  //} else {
  //  Serial.println("bossfight.wav does not exist");
  //}

  randomSeed(generateRandomSeed());

  trainFemaleMarkov();
  
  // Assign a randomly generated name to the damsel
  damsel[0].name = generateFemaleName();

  display.begin();
  u8g2_for_adafruit_gfx.begin(display);
  u8g2_for_adafruit_gfx.setFont(u8g2_font_profont10_mf);
  u8g2_for_adafruit_gfx.setForegroundColor(15);
  display.setContrast(100);

  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_START_PIN, INPUT_PULLUP);

  // Initialize the game
  resetGame();
  playRawSFX(11);
}

void loop() {

  serviceRawSFX();

  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime >= frameDelay) {
    lastUpdateTime = currentTime;
    updateButtonStates();
    if (playerHP > 0) {
      handleUIStateTransitions();
      if (!statusScreen) {
        switch (currentUIState) {
          case UI_NORMAL:
            renderGame();
            updateGame();
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

          case UI_RIDDLE:
            handleRiddles();
            break;
        }
      } else {
        showStatusScreen();
      }
    } else {
      gameOver();
    }
  }
}

void updateGame() {
  updateScreenShake();
  handleInput();
  
  // Only update game state if the player has taken an action
  if (playerActed) {
    handleHungerAndEffects();
    updateScrolling(viewportWidth, viewportHeight, scrollSpeed, offsetX, offsetY);
    updateDamsel();
    updateEnemies();
    updateProjectiles();
  }
}

void renderGame() {
  display.clearDisplay();
  if (!blinded) {
    renderDungeon();
    renderDamsel();
    renderEnemies();
    renderProjectiles();
  }
  renderPlayer();
  renderUI();
  handleDialogue();
  if (playerActed) {
    updateAnimations();
  }
  display.display();
}

int page = 1;
void gameOver() {
  u8g2_for_adafruit_gfx.setFont(u8g2_font_profont10_mf);
  if (showDeathScreen) {
    display.clearDisplay();
    u8g2_for_adafruit_gfx.setCursor(0, 125);
    if (deathCause == "blob") {
      display.drawBitmap(0, 0, wizardDeath_blob, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      u8g2_for_adafruit_gfx.print(F("Slain by a blob!"));
    } else if (deathCause == "batguy") {
      display.drawBitmap(0, 0, wizardDeath_batguy, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      u8g2_for_adafruit_gfx.print(F("Slain by a batguy!"));
    } else if (deathCause == "succubus") {
      display.drawBitmap(0, 0, wizardDeath_succubus, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      u8g2_for_adafruit_gfx.print(F("Slain by a succubus!"));
    } else if (deathCause == "shooter") {
      display.drawBitmap(-10, 0, wizardDeath_shooter, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      u8g2_for_adafruit_gfx.print(F("Slain by a shooter!"));
    } else if (deathCause == "hunger") {
      display.drawBitmap(0, 0, wizardDeath_hunger, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      u8g2_for_adafruit_gfx.print(F("You starved!"));
    } else if (deathCause == "stupidity") {
      display.drawBitmap(0, 0, wizardDeath_stupidity, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      u8g2_for_adafruit_gfx.print(F("You died of pure stupidity."));
    } else {
      u8g2_for_adafruit_gfx.print(F("Yeah, idk what killed you."));
    }
    display.display();
    if ((buttons.bPressed && !buttons.bPressedPrev) || (buttons.aPressed && !buttons.aPressedPrev)) {
      showDeathScreen = false;
    }
    return;
  }

  if (buttons.aPressed && !buttons.aPressedPrev) {
    playRawSFX(8);
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
    if (dungeon == 1) {
      display.print("get out lil bro");
    } else if (dungeon == 2) {
      display.print("lol noob xD");
    } else if (dungeon == 3) {
      display.print("lame :/");
    } else if (dungeon == 4) {
      display.print("meh :(");
    } else if (dungeon == 5) {
      display.print("not bad.");
    } else if (dungeon == 6) {
      display.print("pretty good.");
    } else if (dungeon == 7) {
      display.print("unlucky bro");
    } else if (dungeon == 8) {
      display.print("aw man.");
    } else if (dungeon == 9) {
      display.print("so close!");
    } else if (dungeon == 10) {
      display.print("heck yeah.");
    } else if (dungeon > 10) {
      display.print("how...?");
    }

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
    resetGame();
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
      showDialogue = false;
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