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
int creditsBrightness = 15;
int noiseLevelDiffuseTimer = 0;

// Timing variables
unsigned long lastUpdateTime = 0;
const unsigned long frameDelay = 20; // Update every 100ms

// SD card chip select pin for Teensy Audio Board
const int SD_CS = BUILTIN_SDCARD;  // For Teensy 4.1 with built-in SD slot

void resetGame() {
  //amp1.gain(0.01);
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);

  playWav1.stop();// Stop any currently playing music
  // Reset player stats
  playerHP = 100;
  playerFood = 100;
  dungeon = 1;
  levelOfDamselDeath = -4;
  kills = 0;
  finalStatusScreen = false;
  credits = false;
  bossState = Idle;
  bossStateTimer = 0;
  creditsBrightness = 15;
  nearSuccubus = false;
  succubusIsFriend = false;
  
  // Reset damsel
  damsel[0].name = generateFemaleName();
  damsel[0].levelOfLove = 0;
  knowsDamselName = false;
  carryingDamsel = false;
  
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
  generateDungeon(false);
  spawnEnemies(false);
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
    if (!credits) {
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
    } else {
      renderCredits();
    }
  }
}

void updateGame() {
  updateScreenShake();
  handleInput();
  
  // Only update game state if the player has taken an action
  if (playerActed || playerNearClockEnemy) {
    handleAmbientNoiseLevel();
    handleHungerAndEffects();
    updateScrolling(viewportWidth, viewportHeight, scrollSpeed, offsetX, offsetY);
    updateDamsel();
    updateProjectiles();
    if (dungeon == bossfightLevel) {
      updateBossfight();
    }
  }
  updateEnemies();
}

void handleAmbientNoiseLevel() {
  if (ambientNoiseLevel > 0) {
    noiseLevelDiffuseTimer++;
  }
  if (noiseLevelDiffuseTimer >= 30) {
    ambientNoiseLevel--;
    noiseLevelDiffuseTimer = 0;
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
  if (playerActed || playerNearClockEnemy) {
    updateAnimations();
  }
  display.display();
}

void renderCredits() {
  if (!playWav1.isPlaying()) {
    creditsBrightness -= (creditsBrightness == 0 ? 0 : 1);
  }
  display.clearDisplay();
  if (creditsBrightness > 0) {
    if ((!damsel[0].dead || damsel[0].active) && !succubusIsFriend) {
      display.drawBitmap(0, 0, creditsDamselSaved, SCREEN_WIDTH, SCREEN_HEIGHT, creditsBrightness);
    } else if ((damsel[0].dead || !damsel[0].active) && !succubusIsFriend) {
      display.drawBitmap(0, 0, creditsDamselNotSaved, SCREEN_WIDTH, SCREEN_HEIGHT, creditsBrightness);
    } else if (succubusIsFriend) {
      display.drawBitmap(0, 0, creditsSuccubus, SCREEN_WIDTH, SCREEN_HEIGHT, creditsBrightness);
    }
  }
  if (creditsBrightness == 0) {
    if (bossStateTimer < 15) {
      bossStateTimer ++;
    }
    display.setTextSize(2);
    display.setCursor(22, 50);
    display.setTextColor(bossStateTimer);
    display.print("The End");
  }
  display.display();
}

int page = 1;
static const char* chosenMessage = nullptr;
void gameOver() {
  u8g2_for_adafruit_gfx.setFont(u8g2_font_profont10_mf);
  
  // Reset chosen message when death screen is dismissed
  if (showDeathScreen && ((buttons.bPressed && !buttons.bPressedPrev) || (buttons.aPressed && !buttons.aPressedPrev))) {
    chosenMessage = nullptr;
  }
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
    } else if (deathCause == "hunger" || deathCause == "poison") {
      display.drawBitmap(0, 0, wizardDeath_hunger, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      u8g2_for_adafruit_gfx.print(F(deathCause == "poison" ? "You died from poison!" : "You starved!"));
    } else if (deathCause == "stupidity") {
      display.drawBitmap(0, 0, wizardDeath_stupidity, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      u8g2_for_adafruit_gfx.print(F("You died of pure stupidity."));
    } else if (deathCause == "boss") {
      display.drawBitmap(0, 0, wizardDeath_boss, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      u8g2_for_adafruit_gfx.print(F("You failed."));
    } else if (deathCause == "brute") {
      display.drawBitmap(0, 0, wizardDeath_stupidity, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      u8g2_for_adafruit_gfx.print(F("Slain."));
    } else {
      u8g2_for_adafruit_gfx.print(F("Yeah, idk what killed you."));
    }
    display.display();
    if ((buttons.bPressed && !buttons.bPressedPrev) || (buttons.aPressed && !buttons.aPressedPrev)) {
      showDeathScreen = false;
    }
    return;
  }

  /*if (buttons.aPressed && !buttons.aPressedPrev) {
    playRawSFX(8);
    page++;
    if (page == 3) {
      page = 1;
    }
  }*/ // Only if there is more data to show

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
    if (chosenMessage == nullptr) {
      switch(dungeon) {
      case 1:
        switch(random(0, 3)) {
          case 0: chosenMessage = "get out lil bro"; break;
          case 1: chosenMessage = "r u even trying"; break;
          case 2: chosenMessage = "bruh"; break;
        }
        break;
      case 2:
        switch(random(0, 3)) {
          case 0: chosenMessage = "lol noob xD"; break;
          case 1: chosenMessage = "nah -_-"; break;
          case 2: chosenMessage = "L bozo"; break;
        }
        break;
      case 3:
        switch(random(0, 3)) {
          case 0: chosenMessage = "lame :/"; break;
          case 1: chosenMessage = "get good bro"; break;
          case 2: chosenMessage = "not even close"; break;
        }
        break;
      case 4:
        switch(random(0, 3)) {
          case 0: chosenMessage = "meh :("; break;
          case 1: chosenMessage = "cringe"; break;
          case 2: chosenMessage = "dumb death tbh"; break;
        }
        break;
      case 5:
        switch(random(0, 3)) {
          case 0: chosenMessage = "not bad."; break;
          case 1: chosenMessage = "oof"; break;
          case 2: chosenMessage = "man"; break;
        }
        break;
      case 6:
        switch(random(0, 3)) {
          case 0: chosenMessage = "pretty good."; break;
          case 1: chosenMessage = "take a break."; break;
          case 2: chosenMessage = "long way to go"; break;
        }
        break;
      case 7:
        switch(random(0, 3)) {
          case 0: chosenMessage = "unlucky bro"; break;
          case 1: chosenMessage = "skill issue"; break;
          case 2: chosenMessage = "crazy"; break;
        }
        break;
      case 8:
        switch(random(0, 3)) {
          case 0: chosenMessage = "aw man."; break;
          case 1: chosenMessage = "aww, did u die?"; break;
          case 2: chosenMessage = "splat"; break;
        }
        break;
      case 9:
        switch(random(0, 3)) {
          case 0: chosenMessage = "good run"; break;
          case 1: chosenMessage = "but why..."; break;
          case 2: chosenMessage = "aw."; break;
        }
        break;
      case 10:
        switch(random(0, 3)) {
          case 0: chosenMessage = "noooo"; break;
          case 1: chosenMessage = "*sigh*"; break;
          case 2: chosenMessage = "ur pretty good"; break;
        }
        break;
      default:
        switch(random(0, 3)) {
          case 0: chosenMessage = "very nice."; break;
          case 1: chosenMessage = "good job."; break;
          case 2: chosenMessage = "*salute*"; break;
        }
        break;
    }
    }
    display.print(chosenMessage);

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
    //display.print("[A] next page");
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

  // Only show regular status screens if not final status screen
  if (!finalStatusScreen) {
    if (!succubusIsFriend && !nearSuccubus) {
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
    }
  }
  if (nearSuccubus && !finalStatusScreen) {
    if (!succubusIsFriend) {
      display.drawBitmap(0, 0, succubusFollowScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      u8g2_for_adafruit_gfx.setCursor(0, 115);
      u8g2_for_adafruit_gfx.print(F("Why didn't you kill her?"));
      u8g2_for_adafruit_gfx.setCursor(0, 125);
      u8g2_for_adafruit_gfx.print(F("She tried to kill you..."));
    } else {
      display.drawBitmap(0, 0, succubusFollowScreen2, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      u8g2_for_adafruit_gfx.setCursor(0, 120);
      u8g2_for_adafruit_gfx.print(F("The succubus follows."));
    }
  }
  if (finalStatusScreen) {
    if (succubusIsFriend) {
      display.drawBitmap(0, 0, endScreenSuccubus, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      u8g2_for_adafruit_gfx.setCursor(0, 115);
      u8g2_for_adafruit_gfx.print(F("You defeated the master!"));
      u8g2_for_adafruit_gfx.setCursor(0, 125);
      u8g2_for_adafruit_gfx.print(F("Have fun... ;)"));
    } else if (!damsel[0].dead && damsel[0].active) {
      display.drawBitmap(0, 0, endScreenDamsel, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      u8g2_for_adafruit_gfx.setCursor(0, 115);
      u8g2_for_adafruit_gfx.print(F("You defeated the master!"));
      u8g2_for_adafruit_gfx.setCursor(0, 125);
      u8g2_for_adafruit_gfx.print(F("And rescued the damsel!"));
    } else {
      display.drawBitmap(0, 0, aloneWizardScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      u8g2_for_adafruit_gfx.setCursor(0, 115);
      u8g2_for_adafruit_gfx.print(F("You defeated the master!"));
      u8g2_for_adafruit_gfx.setCursor(0, 125);
      u8g2_for_adafruit_gfx.print(F("But are still alone."));
    }
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
      if (nearSuccubus) {
        succubusIsFriend = true;
      }
      dungeon += 1;
      playerDX = 0;
      playerDY = 1;
      statusScreen = false;
      generateDungeon(dungeon == bossfightLevel ? true : false); // Generate a new dungeon
      showDialogue = false;
      for (int i = 0; i < maxProjectiles; i++) {
        projectiles[i].active = false;
      }
      spawnEnemies(dungeon == bossfightLevel ? true : false);

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

      if (rescued && randomChance == 3 && !carryingDamsel && dungeon < bossfightLevel) {
        damselKidnapScreen = true; // Switch to the kidnap screen
        statusScreen = true;       // Keep status screen active
        damselGotTaken = true;
      } else if (rescued && dungeon < bossfightLevel) {
        damsel[0].x = playerX;
        damsel[0].y = playerY - 1;
      }
    }
    if (finalStatusScreen && !credits) {
      credits = true;
      bossStateTimer = 0;
      playWav1.play("endCredits.wav");
    }
  }
}

void updateBossfight() {
  if (credits) {return;}
  static float bossTargetX = enemies[0].x;
  static float bossTargetY = enemies[0].y;
  static float bossVelocityX = 0;
  static float bossVelocityY = 0;
  static float bossChargeSpeed = 0;
  static bool bossIsCharging = false;

  if (bossState != Beaten) {
    bossStateTimer++;
  }
  if (enemies[0].hp <= 100 && enemies[0].hp > 0) {
    bossState = Enraged;
    if (enemies[0].hp == 100) {
      playWav1.play("alternateBossfight.wav");
      enemies[0].hp -= enemies[0].hp == 100 ? 1 : 0; // Prevent replaying sound
    }
  } else if (enemies[0].hp <= 0) {
    playWav1.stop();
    bossState = Beaten;
  }

  if (enemies[0].hp > 0) {
    // Contact damage to player
    if (checkSpriteCollisionWithSprite(playerX, playerY, enemies[0].x, enemies[0].y) ||
                     checkSpriteCollisionWithSprite(playerX, playerY, enemies[0].x + 1, enemies[0].y) ||
                     checkSpriteCollisionWithSprite(playerX, playerY, enemies[0].x, enemies[0].y + 1) ||
                     checkSpriteCollisionWithSprite(playerX, playerY, enemies[0].x + 1, enemies[0].y + 1)) {
      if (bossStateTimer % 20 == 0) {
        playerHP -= enemies[0].damage;
        triggerScreenShake(2, 2);
        checkIfDeadFrom(enemies[0].name);
      }
    }
  }

  // State transitions for non-enraged boss
  if (bossState != Beaten && bossState != Enraged) {
    if (bossStateTimer == 150) {
      bossState = Floating;
      enemies[0].moveAmount = 0.05;
    } else if (bossStateTimer == 1000) {
      bossState = Shooting;
      enemies[0].moveAmount = 0;
    } else if (bossStateTimer == 1700) {
      bossState = Summoning;
      enemies[0].moveAmount = 0;
    } else if (bossStateTimer == 2000) {
      bossState = Shooting;
      enemies[0].moveAmount = 0;
    } else if (bossStateTimer == 2300) {
      bossState = Floating;
      enemies[0].moveAmount = 0.05;
      bossStateTimer = 0; // Reset timer to loop pattern
    }
  } else if (bossState == Enraged) {
    enemies[0].moveAmount = 0.1; // Faster movement in enraged state
  } else if (bossState == Beaten) {
    enemies[0].moveAmount = 0;
    if (!finalStatusScreen) {
      bossStateTimer -= 10;
      if (bossStateTimer < -2000) {
        statusScreen = true;
        finalStatusScreen = true;
        // Clear succubus flags when showing final status screen
        nearSuccubus = false;
      }
    }
  }

  // Boss AI
  switch (bossState) {
    case Idle:
      enemies[0].damage = 20;
      if (currentDialogue != "You've amused me, little wizard. Time to die!") {
        currentDamselPortrait = bossPortraitIdle;
        dialogueTimeLength = 3000;
        currentDialogue = "You've amused me, little wizard. Time to die!";
        showDialogue = true;
      }
      break;

    case Floating: {
      enemies[0].damage = 20;
      if (!playWav1.isPlaying()) {
        if (SD.exists("bossfight.wav")) {
          if (!playWav1.play("bossfight.wav")) {
            Serial.println("Failed to play bossfight.wav");
          }
        } else {
          Serial.println("bossfight.wav not found");
        }
      }

      showDialogue = false;
      dialogueTimeLength = 0;
      
      // Generate new random target position every few seconds
      if (bossStateTimer % 100 == 0) {
        bossTargetX = random(5, mapWidth - 5);
        bossTargetY = random(5, mapHeight - 5);
      }
      
      // Smooth movement towards target
      float dirX = bossTargetX - enemies[0].x;
      float dirY = bossTargetY - enemies[0].y;
      float distance = sqrt(dirX * dirX + dirY * dirY);
      
      if (distance > 0.1) {
        // Normalize direction and apply smooth movement
        dirX /= distance;
        dirY /= distance;
        enemies[0].x += dirX * enemies[0].moveAmount;
        enemies[0].y += dirY * enemies[0].moveAmount;
      }
      
      // Shoot at player occasionally
      if (bossStateTimer % 100 == 0) {
        float shootDirX = playerX - enemies[0].x;
        float shootDirY = playerY - enemies[0].y;
        float shootDistance = sqrt(shootDirX * shootDirX + shootDirY * shootDirY);
        
        if (shootDistance > 0) {
          shootDirX /= shootDistance;
          shootDirY /= shootDistance;
          shootProjectile(enemies[0].x, enemies[0].y, shootDirX, shootDirY, false, 0);
        }
      }
      break;
    }

    case Shooting: {
      enemies[0].damage = 0;
      if (!playWav1.isPlaying()) {
        playWav1.play("bossfight.wav");
      }

      if (bossStateTimer % 20 == 0) {
        // Calculate direction to player
        float dirX = playerX - enemies[0].x;
        float dirY = playerY - enemies[0].y;
        float distance = sqrt(dirX * dirX + dirY * dirY);
        
        if (distance > 0) {
          // Normalize direction
          dirX /= distance;
          dirY /= distance;
          
          // Shoot projectile from enemy position towards player
          shootProjectile(enemies[0].x, enemies[0].y, dirX, dirY, false, 0);
        }
      }
      break;
    }

    case Enraged: {
      enemies[0].damage = 40;
      if (!playWav1.isPlaying()) {
        if (SD.exists("alternateBossfight.wav")) {
          if (!playWav1.play("alternateBossfight.wav")) {
            Serial.println("Failed to play alternateBossfight.wav");
          }
        } else {
          Serial.println("alternateBossfight.wav not found");
        }
      }
      if (currentDialogue != "AAGH! DIE, PEST!") {
        currentDamselPortrait = bossPortraitEnraged;
        dialogueTimeLength = 300;
        currentDialogue = "AAGH! DIE, PEST!";
        showDialogue = true;
      }
      
      float targetDirX = playerX - enemies[0].x;
      float targetDirY = playerY - enemies[0].y;
      float targetDistance = sqrt(targetDirX * targetDirX + targetDirY * targetDirY);
      
      // State machine for charging behavior
      if (!bossIsCharging) {
        // Prepare to charge - aim at player
        if (targetDistance > 0) {
          targetDirX /= targetDistance;
          targetDirY /= targetDistance;
          
          // Gradually turn towards player while preparing to charge
          float turnSpeed = 0.03;
          bossVelocityX += (targetDirX - bossVelocityX) * turnSpeed;
          bossVelocityY += (targetDirY - bossVelocityY) * turnSpeed;
          
          // Start charging when mostly aimed at player
          float alignmentDot = (bossVelocityX * targetDirX + bossVelocityY * targetDirY);
          if (alignmentDot > 0.9) {
            bossIsCharging = true;
            bossChargeSpeed = enemies[0].moveAmount * 2.5; // Charge faster than normal movement
          }
        }
      } else {
        // Currently charging - check if we've missed the player
        float dotProduct = (targetDirX * bossVelocityX + targetDirY * bossVelocityY) / 
                         sqrt(bossVelocityX * bossVelocityX + bossVelocityY * bossVelocityY);
        
        bool missedTarget = dotProduct < 0; // We've passed the player if dot product is negative
        bool hitWall = checkSpriteCollisionWithTileX(enemies[0].x + bossVelocityX, enemies[0].x, enemies[0].y) ||
                      checkSpriteCollisionWithTileY(enemies[0].y + bossVelocityY, enemies[0].y, enemies[0].x);
        
        if (targetDistance < 1.0 || hitWall || missedTarget) {
          // Apply stronger slowdown when hitting a wall, lighter when just missing
          float slowdownRate = hitWall ? 0.7 : 0.9;
          bossChargeSpeed *= slowdownRate;
          
          if (bossChargeSpeed < 0.02) {
            bossIsCharging = false;
            bossChargeSpeed = 0;
          }
        }
      }
      
      // Apply movement with current charge speed
      float currentSpeed = sqrt(bossVelocityX * bossVelocityX + bossVelocityY * bossVelocityY);
      if (currentSpeed > 0) {
        float moveSpeed = bossIsCharging ? bossChargeSpeed : enemies[0].moveAmount;
        float normalizedVelX = bossVelocityX / currentSpeed;
        float normalizedVelY = bossVelocityY / currentSpeed;
        enemies[0].x += normalizedVelX * moveSpeed;
        enemies[0].y += normalizedVelY * moveSpeed;
      }
      
      // Shoot more frequently when enraged, in direction of movement
      if (bossStateTimer % 10 == 0 && currentSpeed > 0) {
        shootProjectile(enemies[0].x, enemies[0].y, bossVelocityX/currentSpeed, bossVelocityY/currentSpeed, false, 0);
      }
      break;
    }

    case Summoning: {
      enemies[0].damage = 0;
      if (!playWav1.isPlaying()) {
        playWav1.play("bossfight.wav");
      }

      // Summon minions every 60 frames (about once per second)
      if (bossStateTimer % 60 == 0) {
        // Find an empty enemy slot
        for (int j = (succubusIsFriend ? 2 : 1); j < maxEnemies; j++) { // Start from 2 to skip boss and succubus
          if (enemies[j].hp <= 0) {
            // Random position around the boss in a 5-tile radius
            float angle = random(0, 628) / 100.0; // Random angle in radians (0 to 2π)
            float radius = random(2, 5); // Random distance between 2 and 5 tiles
            float spawnX = enemies[0].x + cos(angle) * radius;
            float spawnY = enemies[0].y + sin(angle) * radius;
            
            // Make sure spawn position is valid
            int tileX = round(spawnX);
            int tileY = round(spawnY);
            if (tileX >= 0 && tileX < mapWidth && tileY >= 0 && tileY < mapHeight && 
                dungeonMap[tileY][tileX] == Floor) {
              
              // Pick a random enemy type
              int enemyType = random(0, 9);
              String enemyName;
              switch (enemyType) {
                case 0: enemyName = "blob"; break;
                case 1: enemyName = "blob"; break;
                case 2: enemyName = "blob"; break;
                case 3: enemyName = "blob"; break;
                case 4: enemyName = "batguy"; break;
                case 5: enemyName = "batguy"; break;
                case 6: enemyName = "shooter"; break;
                case 7: enemyName = "shooter"; break;
                case 8: enemyName = "shooter"; break;
              }
              
              if (enemyName == "blob") {
                enemies[j] = { (float)tileX, (float)tileY, 20, false, 0.05, "blob", 20, 2, false, 0, 0, false, false };
                enemies[j].sprite = blobAnimation[random(0, blobAnimationLength)].frame;
              } else if (enemyName == "shooter") {
                enemies[j] = { (float)tileX, (float)tileY, 15, false, 0.06, "shooter", 20, 0, false, 0, 0, false, false };
                enemies[j].sprite = shooterAnimation[random(0, shooterAnimationLength)].frame;
              } else if (enemyName == "batguy") {
                enemies[j] = { (float)tileX, (float)tileY, 10, false, 0.08, "batguy", 20, 1, false, 0, 0, false, false };
                enemies[j].sprite = batguyAnimation[random(0, batguyAnimationLength)].frame;
              }
              
              // Play teleport sound for each spawn
              playRawSFX3D(14, enemies[j].x, enemies[j].y);
              break;
            }
          }
        }
      }
      break;
    }

    case Beaten:
      // Boss is defeated, just stay in place
      if (succubusIsFriend) {
        dialogueTimeLength = 300;
        currentDamselPortrait = succubusPortrait;
        currentDialogue = "Heh... that was quite exhilarating.";
        showDialogue = true;
      } else if (damsel[0].active && !damsel[0].dead) {
        if (currentDialogue != "You did it! You killed him!") {
          currentDamselPortrait = damselPortraitNormal;
          dialogueTimeLength = 300;
          playRawSFX(18);
          currentDialogue = "You did it! You killed him!";
          showDialogue = true;
        }
      }
      break;

    default:
      break;
  }
}