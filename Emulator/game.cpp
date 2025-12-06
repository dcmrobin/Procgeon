#include "game.h"
#include "Adafruit_SSD1327_emu.h"
#include "Adafruit_GFX_emu.h"
#include "Sprites.h"
#include "Dungeon.h"
#include "HelperFunctions.h"
#include "Entities.h"
#include "Item.h"
#include "Inventory.h"
#include "Player.h"
#include "GameAudio.h"
#include "Puzzles.h"
#include "Translation.h"
#include <string.h>
#include "SaveLogic.h"

#include <fstream>
#include <sstream>

// Forward declarations for highscore helpers (defined later)
static void readHighscoresFromFile(int &dngnHighscore, int &kllHighscore);
static void writeHighscoresToFile(int dngnHighscore, int kllHighscore);
bool itemResultScreenActive = false;

unsigned int dngnHighscoreAddress = 0;
unsigned int killHighscoreAddress = 1;
int creditsBrightness = 15;
int noiseLevelDiffuseTimer = 0;
int introNum = 0;

// Timing variables
unsigned long lastUpdateTime = 0;
const unsigned long frameDelay = 18;
bool deleteSV = false;

// SD card chip select pin for Teensy Audio Board
//const int SD_CS = BUILTIN_SDCARD;  // For Teensy 4.1 with built-in SD slot

void resetGame() {
  setJukeboxVolume(0.0f);
  deleteSV = false;
  introNum = 0;
  snprintf(damselDeathMsg, sizeof(damselDeathMsg), "%s", "You killed ");
  DIDNOTRESCUEDAMSEL = false;
  shouldRestartGame = false;
  keysCount = 0;
  //amp1.gain(0.01);
  //pinMode(8, OUTPUT);
  //digitalWrite(8, HIGH);

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
  endlessMode = false;
  
  // Reset damsel
  generateFemaleName(damsel[0].name, sizeof(damsel[0].name));
  damsel[0].levelOfLove = 0;
  knowsDamselName = false;
  damsel[0].beingCarried = false;
  damsel[0].completelyRescued = false;
  damselGotTaken = false;
  
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
  swiftnessRingsNumber = 0;
  strengthRingsNumber = 0;
  weaknessRingsNumber = 0;
  hungerRingsNumber = 0;
  regenRingsNumber = 0;
  lastPotionSpeedModifier = 0;
  playerAttackDamage = 10;
  sicknessRingsNumber = 0;
  aggravateRingsNumber = 0;
  armorRingsNumber = 0;
  indigestionRingsNumber = 0;
  teleportRingsNumber = 0;
  invisibleRingsNumber = 0;

  // Generate new dungeon and spawn enemies
  generateDungeon(false);
  spawnEnemies(false);
}

void game_setup() {
  ////Serial.begin(9600);
  //while (!//Serial && millis() < 4000); // Wait for //Serial Monitor
  //if (CrashReport) {
  //  //Serial.print(CrashReport);
  //}

  initAudio();

  // Initialize SD card
  if (!SD.begin(SD_CS)) {
    ////Serial.println("SD initialization failed!");
    while (1);  // Stop execution
  }
  ////Serial.println("SD initialization done.");

  // Play a sound effect from memory
  if (!loadSFXtoRAM()) {
    ////Serial.println("Failed to load SFX to RAM");
  } else {
    ////Serial.println("SFX loaded successfully");
  }

  ////Serial.println("type 8: teleport damsel to player if damsel is available");
  ////Serial.println("type 7: make tile player is on into the exit");
  ////Serial.println("type 6: add potion to inventory");
  ////Serial.println("type 5: make tile player is on into a riddlestone");
  ////Serial.println("type 4: make tile player is on into a mushroom");
  ////Serial.println("type 3: make tile player is on into an armor");
  ////Serial.println("type 2: make tile player is on into a scroll");
  ////Serial.println("type 1: make tile player is on into a ring");

  // Play a WAV file
  /*if (playWav1.play("bossfight.wav")) {
    //Serial.println("bossfight.wav played successfully");
  } else {
    //Serial.println("bossfight.wav failed to play");
  }

  if (!playWav1.isPlaying()) {
      //Serial.println("bossfight.wav is not playing");
  } else {
      //Serial.println("bossfight.wav is playing");
  }*/

  //if (SD.exists("bossfight.wav")) {
  //  //Serial.println("bossfight.wav does exist");
  //} else {
  //  //Serial.println("bossfight.wav does not exist");
  //}
  worldSeed = generateRandomSeed();
  randomSeed(worldSeed);

  trainFemaleMarkov();
  
  // Assign a randomly generated name to the damsel
  generateFemaleName(damsel[0].name, sizeof(damsel[0].name));

  display.begin();
  //display.begin(display);
  //display.setFont(u8g2_font_profont10_mf);
  //display.setForegroundColor(15);
  display.setContrast(100);

  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_START_PIN, INPUT_PULLUP);
  display.clearDisplay();

  // Initialize the game
  resetGame();
  //currentUIState = UI_SPLASH;
  currentUIState = UI_INTRO;
  //playRawSFX(11);
}

void game_loop() {
  serviceRawSFX();

  if (shouldRestartGame) {
    resetGame();
    shouldRestartGame = false;
  }

  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime >= frameDelay) {
    lastUpdateTime = currentTime;
    updateButtonStates();
    if (!credits) {
      if (playerHP > 0 || currentUIState == UI_RIDDLE) {  // Allow riddle UI even when playerHP <= 0
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

            case UI_SPLASH:
              renderSplashScreen();
              break;

            case UI_INTRO:
              renderIntroScreen();
              break;

            case UI_SECRET:
              renderSecretScreen();
              break;

            case UI_PICROSS:
              updatePicrossPuzzle();
              if (buttons.aPressed && !buttons.aPressedPrev) {
                // Player pressed A to back out
                puzzleFinished = true;
                puzzleSuccess = false;
                pendingChestActive = false;
                currentUIState = UI_NORMAL;
              }
              break;

            case UI_LIGHTSOUT:
              updateLightsOutPuzzle();
              if (buttons.aPressed && !buttons.aPressedPrev) {
                // Player pressed A to back out
                puzzleFinished = true;
                puzzleSuccess = false;
                pendingChestActive = false;
                currentUIState = UI_NORMAL;
              }
              break;
          }
        } else {
          showStatusScreen();
        }
      } else {
        if (!deleteSV) {
          deleteSave();
          deleteSV = true;
        }
        gameOver();
      }
    } else {
      renderCredits();
    }
  }

  if (!playWav2.isPlaying()) {
    playWav2.play("./Audio/12_8.wav");
  }
  if (currentUIState == UI_PAUSE) {
    setJukeboxVolume(0.0f);
  }
}

void updateGame() {
  updateScreenShake();
  handleInput();
  
  // Camera scrolling runs every frame, independent of player action
  updateScrolling(viewportWidth, viewportHeight, scrollSpeed, offsetX, offsetY);

  updateParticles();
  
  // Only update game state if the player has taken an action
  if (playerActed || playerNearClockEnemy) {
    handleAmbientNoiseLevel();
    handleHungerAndEffects();
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
    renderParticles();
  }
  renderPlayer();
  renderUI();
  handleDialogue();
  if (playerActed || playerNearClockEnemy) {
    updateAnimations();
  }
  display.display();
}

void renderIntroScreen() {
  display.setFont(Adafruit_GFX::builtin_font);
  introNum++;
  if (introNum > 50) {
    introNum = 50;
  }
  display.clearDisplay();
  display.setTextColor(15, 0);
  display.setTextSize(1);
  display.setCursor(45, 50);
  display.print("Paladin");
  display.setCursor(42, 60);
  display.print("Presents");
  display.display();
  if (!playWav1.isPlaying() && introNum <= 10) {
    playWav1.play("./Audio/intro.wav");
  }
  if (!playWav1.isPlaying() && introNum > 10) {
    playWav1.stop();
    currentUIState = UI_SPLASH;
  }
}

void renderSecretScreen() {
  display.clearDisplay();
  display.setTextColor(15, 0);
  display.setCursor(1, 7);
  display.print("Hah! You bet I had to add the Konami sequence. Minus the start button. Anyway, yeah here's some hints. Equip the Riddle Stone. Read some of the scrolls right after drinking a See-All potion. Try leading a succubus through an exit. Lastly, don't try to see if the washer fits on your finger. Just don't.");
  //drawWrappedText(1, 7, 128, "Hah! You bet I had to add the Konami sequence. Minus the start button. Anyway, yeah here's some hints. Equip the Riddle Stone. Read some of the scrolls right after drinking a See-All potion. Lastly, don't try to see if the washer fits on your finger. Just don't.");
  display.display();
}

void renderSplashScreen() {
  // KONAMI CODE CHECK
  KonamiInput expected = konamiCode[konamiIndex];
  bool matched = false;

  switch (expected) {
    case K_UP:
      matched = (buttons.upPressed && !buttons.upPressedPrev);
      break;
    case K_DOWN:
      matched = (buttons.downPressed && !buttons.downPressedPrev);
      break;
    case K_LEFT:
      matched = (buttons.leftPressed && !buttons.leftPressedPrev);
      break;
    case K_RIGHT:
      matched = (buttons.rightPressed && !buttons.rightPressedPrev);
      break;
    case K_B:
      matched = (buttons.bPressed && !buttons.bPressedPrev);
      break;
    case K_A:
      matched = (buttons.aPressed && !buttons.aPressedPrev);
      break;
    case K_START:
      matched = (buttons.startPressed && !buttons.startPressedPrev);
      break;
  }

  if (matched) {
    konamiIndex++;

    if (konamiIndex >= konamiLength) {
      currentUIState = UI_SECRET;   // success
      konamiIndex = 0;              // reset
    }
  } 
  else {
    // reset if any other button is pressed
    if ((buttons.upPressed && !buttons.upPressedPrev) ||
        (buttons.downPressed && !buttons.downPressedPrev) ||
        (buttons.leftPressed && !buttons.leftPressedPrev) ||
        (buttons.rightPressed && !buttons.rightPressedPrev) ||
        (buttons.aPressed && !buttons.aPressedPrev) ||
        (buttons.bPressed && !buttons.bPressedPrev) ||
        (buttons.startPressed && !buttons.startPressedPrev)) {

      konamiIndex = 0;
    }
  }

  if (!playWav1.isPlaying()) {
    playWav1.play("./Audio/title_screen.wav");
  }

  display.clearDisplay();
  display.drawBitmap(0, 0, splashScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
  display.setCursor(61, 110);
  display.print("[ENTER]");
  display.display();
}

void renderCredits() {
  if (!playWav1.isPlaying()) {
    creditsBrightness -= (creditsBrightness == 0 ? 0 : 1);
  }
  display.clearDisplay();
  if (creditsBrightness > 0) {
    if ((!damsel[0].dead && damsel[0].active) && !succubusIsFriend && !DIDNOTRESCUEDAMSEL) {
      display.drawBitmap(0, 0, creditsDamselSaved, SCREEN_WIDTH, SCREEN_HEIGHT, creditsBrightness);
    } else if (DIDNOTRESCUEDAMSEL && !succubusIsFriend) {
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
    display.setTextColor(bossStateTimer, 0);
    display.print("The End");
  }
  display.display();
}

int page = 1;
static const char* chosenMessage = nullptr;
void gameOver() {
  // Reset chosen message when death screen is dismissed
  if (showDeathScreen && ((buttons.bPressed && !buttons.bPressedPrev) || (buttons.aPressed && !buttons.aPressedPrev))) {
    chosenMessage = nullptr;
  }
  if (showDeathScreen) {
    display.clearDisplay();
    display.setCursor(0, 117);
    if (strcmp(deathCause, "blob") == 0) {
      display.drawBitmap(0, 0, wizardDeath_blob, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      display.print("Slain by a blob!");
    } else if (strcmp(deathCause, "batguy") == 0) {
      display.drawBitmap(0, 0, wizardDeath_batguy, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      display.print("Slain by a batguy!");
    } else if (strcmp(deathCause, "succubus") == 0) {
      display.drawBitmap(0, 0, wizardDeath_succubus, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      display.print("Slain by a succubus!");
    } else if (strcmp(deathCause, "shooter") == 0) {
      display.drawBitmap(-10, 0, wizardDeath_shooter, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      display.print("Slain by a shooter!");
    } else if (strcmp(deathCause, "hunger") == 0 || strcmp(deathCause, "poison") == 0) {
      display.drawBitmap(0, 0, wizardDeath_hunger, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      display.print(strcmp(deathCause, "poison") == 0 ? "You died from poison!" : "You starved!");
    } else if (strcmp(deathCause, "stupidity") == 0) {
      display.drawBitmap(0, 0, wizardDeath_stupidity, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      display.setCursor(0, 107);
      display.print("You died of pure stupidity.");
    } else if (strcmp(deathCause, "boss") == 0) {
      display.drawBitmap(0, 0, wizardDeath_boss, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      display.print("You failed.");
    } else {
      display.print("Yeah, idk what killed you.");
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
  // Read highscores from file and update if current values are greater
  int dngnHighscore = 0;
  int kllHighscore = 0;
  readHighscoresFromFile(dngnHighscore, kllHighscore);

  bool wroteHighscore = false;
  if (dungeon > dngnHighscore) {
    dngnHighscore = dungeon;
    wroteHighscore = true;
  }
  if (kills > kllHighscore) {
    kllHighscore = kills;
    wroteHighscore = true;
  }
  if (wroteHighscore) writeHighscoresToFile(dngnHighscore, kllHighscore);

  char DHighscore[7];
  snprintf(DHighscore, sizeof(DHighscore), "%d", dngnHighscore);
  char KHighscore[7];
  snprintf(KHighscore, sizeof(KHighscore), "%d", kllHighscore);

  display.clearDisplay();

  display.setCursor(7, 10);
  display.setTextSize(2);
  display.print("Gameover!");
  display.setTextSize(1);
  display.setCursor(5, 30);
  display.print("Press [X]");

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
    deleteSV = false;
  }
}

bool leftDamsel = false;
void showStatusScreen() {
  static bool damselKidnapScreen = false; // Tracks if we are showing the kidnap screen

  display.clearDisplay();

  //display.setFont(u8g2_font_profont10_mf);

  // Only show regular status screens if not final status screen
  if (!nearSuccubus && !endlessMode) {
    if (!finalStatusScreen) {
      if (!succubusIsFriend && !nearSuccubus) {
        if (!damselKidnapScreen) {
          if (dungeon > levelOfDamselDeath + 3) {
            if (!damsel[0].dead && damsel[0].followingPlayer) {
              if (!damsel[0].beingCarried) {
                display.drawBitmap(0, -15, rescueDamselScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
              } else {
                display.drawBitmap(0, -15, carryDamselScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
              }
              display.setCursor(0, 107);
              display.print("You rescued the Damsel!");
            } else {
              display.setCursor(0, 117);
              display.print("Error.");
            }
          } else if (dungeon == levelOfDamselDeath) {
            if (damsel[0].dead) {
              display.drawBitmap(0, -15, deadDamselScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
              display.setCursor(0, 77);
              if (!knowsDamselName) {
                display.print("The Damsel died!");
              } else {
                char msg[130];
                snprintf(msg, sizeof(msg), "%s%s!", damselDeathMsg, damsel[0].name);
                display.print(msg);
              }
              display.setCursor(0, 107);
              display.print(damsel[0].levelOfLove >= 2 ? "She trusted you!" : "How could you!");
              if (damsel[0].levelOfLove >= 5) {
                display.setCursor(0, 117);
                display.print("She loved you!");
              }
            } else if (!damsel[0].dead && !damsel[0].followingPlayer && !damsel[0].beingCarried) {
              display.drawBitmap(0, 0, leftDamselScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
              display.setCursor(0, 117);
              if (!knowsDamselName) {
                display.print("You left the Damsel!");
              } else {
                char msg[100];
                snprintf(msg, sizeof(msg), "You left %s!", damsel[0].name);
                display.print(msg);
              }
              leftDamsel = true;
            }
          } else {
            display.drawBitmap(0, 0, aloneWizardScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
            display.setCursor(0, 117);
            display.print("You progress. Alone.");
          }
        } else {
          display.drawBitmap(0, 0, capturedDamselScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
          display.setCursor(0, 2);
          display.print("The Damsel was captured!");
        }
      }
    }
  } else if (endlessMode) {
    display.drawBitmap(0, 0, aloneWizardScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
    display.setCursor(0, 117);
    display.print("You progress. Alone.");
  }
  if (nearSuccubus && !finalStatusScreen) {
    if (!succubusIsFriend) {
      display.drawBitmap(0, 0, succubusFollowScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      display.setCursor(0, 117);
      display.print("Why didn't you kill her?");
      display.setCursor(0, 117);
      display.print("She tried to kill you...");
    } else {
      display.drawBitmap(0, 0, succubusFollowScreen2, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      display.setCursor(0, 118);
      display.print("The succubus follows.");
    }
  }
  if (finalStatusScreen) {
    if (succubusIsFriend) {
      display.drawBitmap(0, 0, endScreenSuccubus, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      display.setCursor(0, 107);
      display.print("You defeated the master!");
      display.setCursor(0, 117);
      display.print("Have fun... ;)");
    } else if (!damsel[0].dead && damsel[0].active) {
      display.drawBitmap(0, 0, endScreenDamsel, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      display.setCursor(0, 107);
      display.print("You defeated the master!");
      display.setCursor(0, 117);
      display.print("And rescued the damsel!");
    } else {
      display.drawBitmap(0, 0, aloneWizardScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
      display.setCursor(0, 107);
      display.print("You defeated the master!");
      display.setCursor(0, 117);
      display.print("But are still alone.");
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
      bool wasBeingCarried = damsel[0].beingCarried; // Save carry state before reset
      
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
      damsel[0].levelOfLove += rescued && wasBeingCarried ? 1 : 0;
      
      // Preserve carry state across level transition except when entering the bossfight
      if (dungeon == bossfightLevel) {
        damsel[0].beingCarried = false; // In bossfight the damsel should be placed in her cell
      } else {
        damsel[0].beingCarried = wasBeingCarried;
        if (wasBeingCarried) {
          damsel[0].followingPlayer = true; // Ensure followingPlayer reflects carried state
          damsel[0].active = true; // Ensure damsel is active when being carried
        }
      }
      
      /*//Serial.print("DEBUG: rescued=");
      //Serial.print(rescued);
      //Serial.print(", followingPlayer=");
      //Serial.print(damsel[0].followingPlayer);
      //Serial.print(", active=");
      //Serial.print(damsel[0].active);
      //Serial.print(", dead=");
      //Serial.print(damsel[0].dead);
      //Serial.print(", levelOfLove=");
      //Serial.println(damsel[0].levelOfLove);*/
      
      damselGotTaken = rescued ? false : damselGotTaken;
      if (damsel[0].dead) {
        damsel[0].levelOfLove = 0;
        knowsDamselName = false;
        generateFemaleName(damsel[0].name, sizeof(damsel[0].name));
      }
      if (leftDamsel) {
        damsel[0].levelOfLove = 0;
        knowsDamselName = false;
        generateFemaleName(damsel[0].name, sizeof(damsel[0].name));
        leftDamsel = false;
      }

      if (rescued && randomChance == 3 && !wasBeingCarried && dungeon < bossfightLevel) {
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
      playWav1.stop();
      bool played = playWav1.play("./Audio/endCredits.wav");
      ////Serial.print("DEBUG: play endCredits.wav returned ");
      ////Serial.println(played);
      if (!played) {
        ////Serial.println("DEBUG: endCredits.wav failed to start (file missing or busy)");
      }
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
    if (bossState != Enraged) {
      playWav1.stop();
      playWav1.play("./Audio/alternateBossfight.wav");
    }
    bossState = Enraged;
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
    dungeonMap[(mapHeight/2) + 4][(mapWidth/2) - 3] = Exit;
    dungeonMap[(mapHeight/2) + 4][(mapWidth/2) + 3] = Freedom;
    /*if (!finalStatusScreen) {
      bossStateTimer -= 10;
      if (bossStateTimer < -2000) {
        statusScreen = true;
        finalStatusScreen = true;
        // Clear succubus flags when showing final status screen
        nearSuccubus = false;
      }
    }*/
  }

  // Boss AI
  switch (bossState) {
    case Idle:
      enemies[0].damage = 20;
      if (strcmp(currentDialogue, "You've amused me, little wizard. Time to die!") != 0) {
        currentDamselPortrait = bossPortraitIdle;
        dialogueTimeLength = 3000;
        snprintf(currentDialogue, sizeof(currentDialogue), "%s", "You've amused me, little wizard. Time to die!");
        showDialogue = true;
      }
      break;

    case Floating: {
      enemies[0].damage = 20;
      if (!playWav1.isPlaying()) {
        if (SD.exists("./Audio/bossfight.wav")) {
          if (!playWav1.play("./Audio/bossfight.wav")) {
            //Serial.println("Failed to play bossfight.wav");
          }
        } else {
          //Serial.println("bossfight.wav not found");
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
          playRawSFX(1);
        }
      }
      break;
    }

    case Shooting: {
      enemies[0].damage = 0;
      if (!playWav1.isPlaying()) {
        playWav1.play("./Audio/bossfight.wav");
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
          playRawSFX(1);
        }
      }
      break;
    }

    case Enraged: {
      enemies[0].damage = 40;
      if (!playWav1.isPlaying()) {
        if (SD.exists("./Audio/alternateBossfight.wav")) {
          if (!playWav1.play("./Audio/alternateBossfight.wav")) {
            //Serial.println("Failed to play alternateBossfight.wav");
          }
        } else {
          //Serial.println("alternateBossfight.wav not found");
        }
      }
      if (strcmp(currentDialogue, "AAGH! DIE, PEST!") != 0) {
        currentDamselPortrait = bossPortraitEnraged;
        dialogueTimeLength = 300;
        snprintf(currentDialogue, sizeof(currentDialogue), "%s", "AAGH! DIE, PEST!");
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
        playRawSFX(1);
      }
      break;
    }

    case Summoning: {
      enemies[0].damage = 0;
      if (!playWav1.isPlaying()) {
        playWav1.play("./Audio/bossfight.wav");
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
              const char* enemyName;
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
                default: enemyName = "blob"; break;
              }
              
              if (strcmp(enemyName, "blob") == 0) {
                enemies[j] = { (float)tileX, (float)tileY, 20, false, 0.05, "blob", 20, 2, false, 0, 0, false, false };
                enemies[j].sprite = blobAnimation[random(0, blobAnimationLength)].frame;
              } else if (strcmp(enemyName, "shooter") == 0) {
                enemies[j] = { (float)tileX, (float)tileY, 15, false, 0.06, "shooter", 20, 0, false, 0, 0, false, false };
                enemies[j].sprite = shooterAnimation[random(0, shooterAnimationLength)].frame;
              } else if (strcmp(enemyName, "batguy") == 0) {
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
        snprintf(currentDialogue, sizeof(currentDialogue), "%s", "Heh... that was quite exhilarating.");
        showDialogue = true;
      } else if (damsel[0].active && !damsel[0].dead) {
        if (strcmp(currentDialogue, "You did it! You killed him!") != 0 && strcmp(currentDialogue, "Please don't go- come be free, free with me!") != 0) {
          currentDamselPortrait = damselPortraitNormal;
          dialogueTimeLength = 300;
          playRawSFX(18);
          snprintf(currentDialogue, sizeof(currentDialogue), "%s", "You did it! You killed him!");
          showDialogue = true;
        }
      }
      break;

    default:
      break;
  }
}

// Highscore file helpers
static void readHighscoresFromFile(int &dngnHighscore, int &kllHighscore) {
  dngnHighscore = 0;
  kllHighscore = 0;
  std::ifstream in("highscore");
  if (!in) return; // file doesn't exist yet
  // Expected format: two integers (dungeonHighscore killHighscore)
  if (!(in >> dngnHighscore)) dngnHighscore = 0;
  if (!(in >> kllHighscore)) kllHighscore = 0;
}

static void writeHighscoresToFile(int dngnHighscore, int kllHighscore) {
  std::ofstream out("highscore", std::ios::trunc);
  if (!out) return; // can't write, silently fail in emulator
  out << dngnHighscore << " " << kllHighscore << "\n";
}