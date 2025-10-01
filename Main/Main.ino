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
  if (playerActed || playerNearClockEnemy) {
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

void updateBossfight() {
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
  } else if (enemies[0].hp <= 0) {
    bossState = Beaten;
  }

  // State transitions for non-enraged boss
  if (bossState != Beaten && bossState != Enraged) {
    if (bossStateTimer == 600) {
      bossState = Floating;
      enemies[0].moveAmount = 0.05;
    } else if (bossStateTimer == 2000) {
      bossState = Shooting;
      enemies[0].moveAmount = 0;
    } else if (bossStateTimer == 3000) {
      bossState = Summoning;
      enemies[0].moveAmount = 0;
    } else if (bossStateTimer == 4000) {
      bossState = Shooting;
      enemies[0].moveAmount = 0;
    } else if (bossStateTimer == 5000) {
      bossState = Floating;
      enemies[0].moveAmount = 0.05;
      bossStateTimer = 0; // Reset timer to loop pattern
    }
  } else if (bossState == Enraged) {
    enemies[0].moveAmount = 0.1; // Faster movement in enraged state
  } else if (bossState == Beaten) {
    enemies[0].moveAmount = 0;
    bossStateTimer -= (bossStateTimer >= 0 ? 1000 : 0);
    if (bossStateTimer < 0) {
      // Handle game ending here
    }
  }

  // Boss AI
  switch (bossState) {
    case Idle:
      showDialogue = true;
      currentDamselPortrait = bossPortraitIdle;
      dialogueTimeLength = 3000;
      currentDialogue = "You've amused me, little wizard. Time to die!";
      break;

    case Floating: {
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
          shootProjectile(enemies[0].x, enemies[0].y, shootDirX, shootDirY, false);
        }
      }
      break;
    }

    case Shooting: {
      if (bossStateTimer % 50 == 0) {
        // Calculate direction to player
        float dirX = playerX - enemies[0].x;
        float dirY = playerY - enemies[0].y;
        float distance = sqrt(dirX * dirX + dirY * dirY);
        
        if (distance > 0) {
          // Normalize direction
          dirX /= distance;
          dirY /= distance;
          
          // Shoot projectile from enemy position towards player
          shootProjectile(enemies[0].x, enemies[0].y, dirX, dirY, false);
        }
      }
      break;
    }

    case Enraged: {
      showDialogue = true;
      currentDamselPortrait = bossPortraitEnraged;
      dialogueTimeLength = 1000;
      currentDialogue = "AAGH! DIE, PEST!";
      
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
        // Currently charging - maintain direction but gradually slow down if hit wall or missed player
        if (targetDistance < 1.0 || checkSpriteCollisionWithTileX(enemies[0].x + bossVelocityX, enemies[0].x, enemies[0].y) ||
            checkSpriteCollisionWithTileY(enemies[0].y + bossVelocityY, enemies[0].y, enemies[0].x)) {
          bossChargeSpeed *= 0.8; // Rapid slowdown when hitting something
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
        shootProjectile(enemies[0].x, enemies[0].y, bossVelocityX/currentSpeed, bossVelocityY/currentSpeed, false);
      }
      break;
    }

    case Summoning: {
      // Boss summons minions
      break;
    }

    case Beaten:
      // Boss is defeated, just stay in place
      break;

    default:
      break;
  }
}