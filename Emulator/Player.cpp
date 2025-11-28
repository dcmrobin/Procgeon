#include "Player.h"
#include "HelperFunctions.h"
#include "Inventory.h"
#include "Dungeon.h"
#include "Entities.h"
#include "GameAudio.h"
#include "Item.h"
#include "Puzzles.h"
#include <string.h>
#include "Translation.h"

char deathCause[50] = "";
char currentDialogue[200] = "";
bool DIDNOTRESCUEDAMSEL = false;
float playerX = 0;
float playerY = 0;
float currentSpeedMultiplier = 1;
int playerHP = 100;
int playerMaxHP = 100;
int speedTimer = 500;
int seeAllTimer = 1000;
int dungeon = 1;
int kills = 0;
int playerDX;
int playerDY;
int ingredient1index = 0;
int playerFood = 100;
int dialogueTimeLength = 1000;
int timeTillNextDialogue = 1000;
bool shouldRestartGame = false;
bool speeding = false;
bool hasMap = false;
bool paused = false;
bool nearSuccubus = false;
bool succubusIsFriend = false;
bool damselGotTaken = false;
bool damselSayThanksForRescue = false;
bool knowsDamselName = false;
bool combiningTwoItems = false;
bool playerMoving = false;
bool starving = false;
bool seeAll = false;
bool showDialogue = false;
GameItem combiningItem1 = {};
GameItem combiningItem2 = {};
bool playerActed = false;  // Initialize the new variable
bool confused = false;  // State for confusion effect
int confusionTimer = 0;  // Timer for confusion effect
bool damselWasFollowing = false;  // Track if damsel was following last frame
int damselWaitUpTimer = 0;  // Timer to prevent spam of "wait up" messages
bool damselSaidWaitUp = false;  // Track if damsel has said "wait up" recently
float equippedArmorValue = 0.0f;  // Current armor value (damage reduction)
GameItem equippedArmor = {};  // Currently equipped armor item
bool equippedRiddleStone = false;
int playerAttackDamage = 10; // Player's attack damage, can be increased by enchant scroll
int swiftnessRingsNumber = 0;
int strengthRingsNumber = 0;
int weaknessRingsNumber = 0;
int hungerRingsNumber = 0;
int regenRingsNumber = 0;
int sicknessRingsNumber = 0;
int aggravateRingsNumber = 0;
int armorRingsNumber = 0;
int indigestionRingsNumber = 0;
int teleportRingsNumber = 0;
int invisibleRingsNumber = 0;
float lastPotionSpeedModifier = 0;
bool ridiculed = false;
int ridiculeTimer = 0;
int lastRidiculeIndex = -1;
bool isRidiculeDialogue = false;
bool glamoured = false;
int glamourTimer = 0;
bool blinded = false;
int blindnessTimer = 0;
bool paralyzed = false;
int paralysisTimer = 0;
bool playerNearClockEnemy = false;
int shootDelay = 0;
bool reloading;
char damselDeathMsg[100] = "You killed ";
bool endlessMode = false;

void renderPlayer() {
  float screenX = (playerX - offsetX) * tileSize;
  float screenY = (playerY - offsetY) * tileSize;

  // Ensure the player is within the viewport
  if (screenX >= 0 && screenX < SCREEN_WIDTH && screenY >= 0 && screenY < SCREEN_HEIGHT) {
    if (invisibleRingsNumber == 0 || seeAll) {
      display.drawBitmap((screenX + tileSize / 2) - tileSize/2, (screenY + tileSize / 2) - tileSize/2, playerSprite, tileSize, tileSize, 15);
    }

    // --- Show 'carry [b]' prompt if player can carry damsel ---
    float dx = playerX - damsel[0].x;
    float dy = playerY - damsel[0].y;
    float distanceSquared = dx * dx + dy * dy;
    if (!damsel[0].beingCarried && !damsel[0].dead && damsel[0].levelOfLove >= 6 && distanceSquared <= 0.4) {
      display.setTextSize(1);
      display.setTextColor(15);
      // Center the text under the player sprite (estimate 6px per char, 9 chars)
      int textWidth = 9 * 6;
      int textX = (screenX + tileSize / 2) - (textWidth / 2);
      int textY = (screenY + tileSize) + 2;
      display.setCursor(textX, textY);
      display.print("Carry [B]");
    }

    // --- Show 'Open Chest [B]' prompt only if player is facing a chest ---
    int facingX = round(playerX) + playerDX;
    int facingY = round(playerY) + playerDY;
    bool facingChest = false;
    bool facingClosedDoor = false;
    bool facingOpenDoor = false;
    bool facingOpenExit = false;
    bool facingFreedom = false;
    if (!(playerDX == 0 && playerDY == 0) && facingX >= 0 && facingX < mapWidth && facingY >= 0 && facingY < mapHeight) {
      facingChest = (dungeonMap[facingY][facingX] == ChestTile);
      facingClosedDoor = (dungeonMap[facingY][facingX] == DoorClosed);
      facingOpenDoor = (dungeonMap[facingY][facingX] == DoorOpen);
      facingOpenExit = (dungeonMap[facingY][facingX] == Exit);
      facingFreedom = (dungeonMap[facingY][facingX] == Freedom);
    }
    if (facingChest || facingClosedDoor || facingOpenDoor || facingOpenExit || facingFreedom) {
      // Prevent player from shooting while opening chest or door
      reloading = true;
      shootDelay = 0;

      display.setTextSize(1);
      display.setTextColor(15);
      int textWidth = 14 * 6; // "Open Chest [B]" is 14 chars
      int textX = (screenX + tileSize / 2) - (textWidth / 2);
      int textY = (screenY + tileSize) + 12;
      display.setCursor(textX, textY);
      if (facingChest) {
        display.print("Open Chest [B]");
      } else if (facingClosedDoor) {
        display.print("Open Door [B]");
      } else if (facingOpenDoor) {
        display.print("Close Door [B]");
      } else if (facingOpenExit) {
        display.print("Descend [B]");
      } else if (facingFreedom) {
        display.print("Escape [B]");
      }
    }
  }

  // Update viewport offset if needed
  if (playerX - offsetX < 2 && offsetX > 0) offsetX -= scrollSpeed;
  if (playerX - offsetX > viewportWidth - 3 && offsetX < mapWidth - viewportWidth) offsetX += scrollSpeed;
  if (playerY - offsetY < 2 && offsetY > 0) offsetY -= scrollSpeed;
  if (playerY - offsetY > viewportHeight - 3 && offsetY < mapHeight - viewportHeight) offsetY += scrollSpeed;
}

int damselHealDelay = 0;
int carryingDelay = 0;
float baseSpeed = 0.1;
void handleInput() {
  float newX = playerX;
  float newY = playerY;
  baseSpeed = 0.1 + (swiftnessRingsNumber * 0.05);// Apply speed effect per ring of swiftness equipped

  float speed = speeding ? baseSpeed * currentSpeedMultiplier : baseSpeed;

  if (speed <= 0) {
    speed = 0.01;
  }

  // Reset playerActed at the start of each input handling
  playerActed = false;

  float diagSpeed = speed * 0.7071;

  // Store the actual button states
  bool upPressed = buttons.upPressed;
  bool downPressed = buttons.downPressed;
  bool leftPressed = buttons.leftPressed;
  bool rightPressed = buttons.rightPressed;

  // Reverse controls if confused
  if (confused) {
    bool temp = upPressed;
    upPressed = downPressed;
    downPressed = temp;
    temp = leftPressed;
    leftPressed = rightPressed;
    rightPressed = temp;
  }

  if (upPressed && !leftPressed && !rightPressed) {
    if (!paralyzed) {
      playerDY = -1;
      playerDX = 0;
      newY -= speed; // Move up
    }
    playerActed = true;
  } else if (downPressed && !leftPressed && !rightPressed) {
    if (!paralyzed) {
      playerDY = 1;
      playerDX = 0;
      newY += speed; // Move down
    }
    playerActed = true;
  } else if (leftPressed && !upPressed && !downPressed) {
    if (!paralyzed) {
      playerDX = -1;
      playerDY = 0;
      playerSprite = damsel[0].beingCarried ? playerCarryingDamselSpriteLeft : playerSpriteLeft;
      newX -= speed; // Move left
    }
    playerActed = true;
  } else if (rightPressed && !upPressed && !downPressed) {
    if (!paralyzed) {
      playerDX = 1;
      playerDY = 0;
      playerSprite = damsel[0].beingCarried ? playerCarryingDamselSpriteRight : playerSpriteRight;
      newX += speed; // Move right
    }
    playerActed = true;
  } else if (upPressed && leftPressed) {
    if (!paralyzed) {
      playerDY = -1;
      playerDX = -1;
      playerSprite = damsel[0].beingCarried ? playerCarryingDamselSpriteLeft : playerSpriteLeft;
      newY -= diagSpeed; // Move up & left
      newX -= diagSpeed; // Move up & left
    }
    playerActed = true;
  } else if (upPressed && rightPressed) {
    if (!paralyzed) {
      playerDY = -1;
      playerDX = 1;
      playerSprite = damsel[0].beingCarried ? playerCarryingDamselSpriteRight : playerSpriteRight;
      newY -= diagSpeed; // Move up & right
      newX += diagSpeed; // Move up & left
    }
    playerActed = true;
  } else if (downPressed && leftPressed) {
    if (!paralyzed) {
      playerDX = -1;
      playerDY = 1;
      playerSprite = damsel[0].beingCarried ? playerCarryingDamselSpriteLeft : playerSpriteLeft;
      newX -= diagSpeed; // Move left & down
      newY += diagSpeed; // Move up & left
    }
    playerActed = true;
  } else if (downPressed && rightPressed) {
    if (!paralyzed) {
      playerDX = 1;
      playerDY = 1;
      playerSprite = damsel[0].beingCarried ? playerCarryingDamselSpriteRight : playerSpriteRight;
      newX += diagSpeed; // Move right & down
      newY += diagSpeed; // Move up & left
    }
    playerActed = true;
  }

  // Check if the player is moving
  playerMoving = buttons.upPressed || buttons.downPressed || buttons.leftPressed || buttons.rightPressed ? true : false;

  float dx = playerX - damsel[0].x;
  float dy = playerY - damsel[0].y;
  float distanceSquared = dx * dx + dy * dy;

  if (buttons.bPressed) {
    playerActed = true;
    if (distanceSquared <= 0.4 && !damsel[0].dead && damsel[0].levelOfLove >= 6) {
      startCarryingDamsel();
    }
  } else {
    carryingDelay = 0;
  }

  if (buttons.bPressed) {
    if (!reloading && !damsel[0].beingCarried && distanceSquared > 0.3) {
      shootProjectile(playerX, playerY, playerDX, playerDY, true, -1); // Shoot in current direction
      playRawSFX(1);
      reloading = true;
      playerActed = true;  // Player has taken an action
    }
  }

  if (playerActed) {
    if (reloading) {
      shootDelay++;
      if (shootDelay >= 10) {
        reloading = false;
        shootDelay = 0;
      }
    }
  }

  if (//Serial.available() > 0) {// for debug purposes
    char input = //Serial.read();
    if (input == '7') {
      setTile((int)playerX, (int)playerY, Exit);
    } else if (input == '8') {
      moveDamselToPos(playerX, playerY);
      if (!damsel[0].active) {
        if (succubusIsFriend) {
          //Serial.println("The damsel is deactivated because succubus is friend.");
        } else {
          //Serial.println("The damsel is not active.");
        }
      }
    } else if (input == '6') {
      addToInventory(getItem(getRandomPotion(random(0, NUM_POTIONS), false)), false);
    } else if (input == '5') {
      setTile((int)playerX, (int)playerY, RiddleStoneTile);
    } else if (input == '4') {
      setTile((int)playerX, (int)playerY, MushroomTile);
    } else if (input == '3') {
      setTile((int)playerX, (int)playerY, ArmorTile);
    } else if (input == '2') {
      setTile((int)playerX, (int)playerY, ScrollTile);
    } else if (input == '1') {
      setTile((int)playerX, (int)playerY, RingTile);
    }
  }

  int rNewX = round(newX);
  int rNewY = round(newY);

  // Check collision with walls
  if (dungeonMap[rNewY][rNewX] == Floor || dungeonMap[rNewY][rNewX] == Exit || dungeonMap[rNewY][rNewX] == StartStairs || dungeonMap[rNewY][rNewX] == Freedom || dungeonMap[rNewY][rNewX] == DoorOpen) {
    playerX = newX;
    playerY = newY;
  } else if (dungeonMap[rNewY][rNewX] == Potion) {
    if (addToInventory(getItem(getRandomPotion(random(6), true)), false)) {
      playRawSFX(3);
      dungeonMap[rNewY][rNewX] = Floor;
    }
  } else if (dungeonMap[rNewY][rNewX] == Map) {
    playRawSFX(3);
    hasMap = true;
    dungeonMap[rNewY][rNewX] = Floor;
  } else if (dungeonMap[rNewY][rNewX] == MushroomTile) {
    if (addToInventory(getItem(Mushroom), false)) {
      playRawSFX(3);
      dungeonMap[rNewY][rNewX] = Floor;
    }
  } else if (dungeonMap[rNewY][rNewX] == RiddleStoneTile) {
    if (addToInventory(getItem(RiddleStone), true)) {
      playRawSFX(3);
      dungeonMap[rNewY][rNewX] = Floor;
    }
  } else if (dungeonMap[rNewY][rNewX] == ArmorTile) {
    GameItems armorTypes[] = { 
        LeatherArmor,
        IronArmor,
        MagicRobe,
        Cloak,
        ChaosArmor,
        RingMailArmor,
        DenimJacket,
        Trenchcoat,
        SpikyArmor
    };

    const int armorCount = sizeof(armorTypes) / sizeof(armorTypes[0]);
    const int maxRarity = 5;

    // 1. Compute total weight using inverted rarity
    int totalWeight = 0;

    for (int i = 0; i < armorCount; i++) {
        int rarity = getItem(armorTypes[i]).rarity;   // 1 to 5
        int weight = (maxRarity + 1) - rarity;        // invert rarity
        totalWeight += weight;
    }

    // 2. Random number 0..totalWeight-1
    int r = random(0, totalWeight);

    GameItems chosenArmor = LeatherArmor; // fallback

    // 3. Walk through weights and choose
    int cumulative = 0;

    for (int i = 0; i < armorCount; i++) {
        int rarity = getItem(armorTypes[i]).rarity;
        int weight = (maxRarity + 1) - rarity;

        cumulative += weight;

        if (r < cumulative) {
            chosenArmor = armorTypes[i];
            break;
        }
    }

    // 4. Add to inventory
    if (addToInventory(getItem(chosenArmor), true)) {
        playRawSFX(3);
        dungeonMap[rNewY][rNewX] = Floor;
    }
  } else if (dungeonMap[rNewY][rNewX] == ScrollTile) {    
    if (addToInventory(getItem(Scroll), false)) {
      playRawSFX(3);
      dungeonMap[rNewY][rNewX] = Floor;
    }
  } else if (dungeonMap[rNewY][rNewX] == RingTile) {    
    if (addToInventory(getItem(Ring), false)) {// the FALSE for canBeCursed is because the only rings that are cursed are the ones with negative effects
      playRawSFX(3);
      dungeonMap[rNewY][rNewX] = Floor;
    }
  }


  int rPx = round(playerX);
  int rPy = round(playerY);

  // --- Open/close door only in the direction the player is facing ---
  if (buttons.bPressed && !buttons.bPressedPrev) {
    int targetDX = playerDX;
    int targetDY = playerDY;
    // If playerDX and playerDY are both zero (no movement yet), fallback to old logic for chests only
    if (targetDX == 0 && targetDY == 0) {
      for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
          int cx = rPx + dx;
          int cy = rPy + dy;
          if (cx >= 0 && cx < mapWidth && cy >= 0 && cy < mapHeight) {
            if (dungeonMap[cy][cx] == ChestTile) {
              playRawSFX(12);
              OpenChest(cy, cx, dy);
              return;
            }
          }
        }
      }
    } else {
      int tx = rPx + targetDX;
      int ty = rPy + targetDY;
      if (tx >= 0 && tx < mapWidth && ty >= 0 && ty < mapHeight) {
        if (dungeonMap[ty][tx] == DoorClosed) {
          // Open the door if it's closed
          dungeonMap[ty][tx] = DoorOpen;
          playRawSFX(12);
        } else if (dungeonMap[ty][tx] == DoorOpen) {
          // Close the door if it's open
          dungeonMap[ty][tx] = DoorClosed;
          playRawSFX(13);
        } else if (dungeonMap[ty][tx] == ChestTile) {
          playRawSFX(12);
          OpenChest(ty, tx, targetDY);
        } else if (dungeonMap[ty][tx] == Exit) {
          playRawSFX(11);
          if (dungeon != bossfightLevel) {
            if (!damsel[0].dead && !damsel[0].followingPlayer && damsel[0].active) {
              levelOfDamselDeath = dungeon;
              damsel[0].active = false;
            }
            statusScreen = true;
          } else {
            if (damsel[0].levelOfLove >= 8 && !damsel[0].dead) {
              // Damsel does not let player go deeper in the dungeon
              currentDamselPortrait = damselPortraitScared;
              snprintf(currentDialogue, sizeof(currentDialogue), "%s", "Please don't go- come be free, free with me!");
              showDialogue = true;
              dialogueTimeLength = 400;
            } else if (damsel[0].levelOfLove < 8) {
              playRawSFX(11);
              statusScreen = true;
              endlessMode = true;
              damsel[0].active = false;
              damsel[0].levelOfLove = 0;
              damsel[0].x = -3000;
              damsel[0].y = -3000;
              damsel[0].beingCarried = false;
              damsel[0].followingPlayer = false;
              damsel[0].completelyRescued = false;
            }
          }
        } else if (dungeonMap[ty][tx] == Freedom) {
          playRawSFX(11);
          statusScreen = true;
          finalStatusScreen = true;
          bossStateTimer = 0;
          // Clear succubus flags when showing final status screen
          nearSuccubus = false;
        }
      }
    }
  }
}

void startCarryingDamsel() {
  carryingDelay += 1;
  display.fillRect(0, 0, carryingDelay, 15, (int)(carryingDelay/8));
  display.display();
  if (carryingDelay >= SCREEN_WIDTH) {
    damsel[0].beingCarried = !damsel[0].beingCarried;

    if (damsel[0].beingCarried) {
      currentDamselPortrait = damselPortraitCarrying;
      dialogueTimeLength = 300;
      snprintf(currentDialogue, sizeof(currentDialogue), "%s", "Oh! Thanks...");
      showDialogue = true;
      playRawSFX(20);
      playerSprite = playerSprite == playerSpriteRight ? playerCarryingDamselSpriteRight : playerCarryingDamselSpriteLeft;
    } else {
      playRawSFX(15);
      playerSprite = playerSprite == playerCarryingDamselSpriteRight ? playerSpriteRight : playerSpriteLeft;
    }

    carryingDelay = 0;
  }
}

void handlePauseScreen() {
  static int pauseSelection = 0; // 0 = Volume, 1 = Restart, 2 = Save, 3 = Load
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(27, 16);
  display.setTextColor(15);
  display.print("PAUSED");
  display.setTextSize(1);

  // Draw menu entries using highlighted background for selection
  int baseY = 46;
  // Volume entry
  if (pauseSelection == 0) {
    display.fillRect(10, baseY - 2, 108, 12, 15); // white background
    display.setTextColor(0); // black text on highlight
  } else {
    display.setTextColor(15); // white text on black
  }
  display.setCursor(18, baseY);
  display.print("Volume:");
  // Draw left chevron, value, right chevron
  int vol = masterVolume; // 1..10
  char volStr[20];
  snprintf(volStr, sizeof(volStr), "<%d>", vol);
  display.setCursor(80, baseY);
  display.print(volStr);

  // Restart entry
  baseY += 14;
  if (pauseSelection == 1) {
    display.fillRect(10, baseY - 2, 108, 12, 15);
    display.setTextColor(0);
  } else {
    display.setTextColor(15);
  }
  display.setCursor(18, baseY);
  display.print("Restart Game");

  baseY += 14;
  if (pauseSelection == 2) {
    display.fillRect(10, baseY - 2, 108, 12, 15);
    display.setTextColor(0);
  } else {
    display.setTextColor(15);
  }
  display.setCursor(18, baseY);
  display.print("Save game");

  baseY += 14;
  if (pauseSelection == 3) {
    display.fillRect(10, baseY - 2, 108, 12, 15);
    display.setTextColor(0);
  } else {
    display.setTextColor(15);
  }
  display.setCursor(18, baseY);
  display.print("Load game");

  // Reset text color to white for footer
  display.setTextColor(15);

  display.setCursor(24, 110);
  display.print("Press [START]");
  display.display();

  // Handle navigation input: up/down to move selection, left/right to change volume
  if (buttons.upPressed && !buttons.upPressedPrev) {
      pauseSelection = (pauseSelection + 3) % 4;  // goes up and wraps from 0 -> 3
      playRawSFX(8);
  }

  if (buttons.downPressed && !buttons.downPressedPrev) {
      pauseSelection = (pauseSelection + 1) % 4;  // goes down and wraps from 3 -> 0
      playRawSFX(8);
  }

  if (pauseSelection == 0) {
    if (buttons.leftPressed && !buttons.leftPressedPrev) {
      masterVolume = max(1, masterVolume - 1);
      float volf = masterVolume / 10.0f;
      sgtl5000_1.volume(volf);
      // Also scale mixer gains so playRawSFX and music levels respect master volume
      mixer1.gain(0, 0.5 * volf);
      mixer1.gain(1, 0.5 * volf);
      mixer1.gain(2, 0.5 * volf);
      mixer1.gain(3, 0.5 * volf);
      musicMixer.gain(0, volf); // SFX master
      musicMixer.gain(1, 0.2 * volf); // music
      playRawSFX(7);
    }
    if (buttons.rightPressed && !buttons.rightPressedPrev) {
      masterVolume = min(10, masterVolume + 1);
      float volf = masterVolume / 10.0f;
      sgtl5000_1.volume(volf);
      mixer1.gain(0, 0.5 * volf);
      mixer1.gain(1, 0.5 * volf);
      mixer1.gain(2, 0.5 * volf);
      mixer1.gain(3, 0.5 * volf);
      musicMixer.gain(0, volf);
      musicMixer.gain(1, 0.2 * volf);
      playRawSFX(7);
    }
  }

  // Restart: press B to confirm
  if (buttons.bPressed && !buttons.bPressedPrev) {
    if (pauseSelection == 1) {
      playRawSFX(9);
      currentUIState = UI_SPLASH;
    } else if (pauseSelection == 2) {
      trySaveGame();
    } else if (pauseSelection == 3) {
      tryLoadGame();
    }
  }
}

int hungerTick = 0;
void handleHungerAndEffects() {
  hungerTick += playerMoving || damsel[0].beingCarried ? 2 : 1;

  if (hungerTick >= (starving ? 70 : 700)) {
    if (starving) {
      playerHP -= 4;
    } else {
      int hungerDrain = 2 + (hungerRingsNumber*7) + (regenRingsNumber*3) - (indigestionRingsNumber*2);
      playerFood -= hungerDrain;
    }
    hungerTick = 0;
    starving = playerFood <= 0 ? true : false;
  }

  checkIfDeadFrom("hunger");

  // Update confusion timer
  if (confused) {
    confusionTimer--;
    if (confusionTimer <= 0) {
      confusionTimer = 1000;
      confused = false;
    }
  }

  if (speeding) {
    speedTimer--;
    if (speedTimer <= 0) {
      speedTimer = 500;
      speeding = false;
      currentSpeedMultiplier -= lastPotionSpeedModifier;
      lastPotionSpeedModifier = 0;
    }
  }

  if (seeAll) {
    seeAllTimer--;
    if (seeAllTimer <= 0) {
      seeAllTimer = 1000;
      seeAll = false;
    }
  }

  if (damsel[0].beingCarried) {
    damselHealDelay++;
    if (damselHealDelay >= 200) {
      damselHealDelay = 0;
      playerHP += damsel[0].levelOfLove;
      if (playerHP > playerMaxHP) playerHP = playerMaxHP;
    }
  }

  handleRingEffects();

  // Update ridicule timer
  if (ridiculed) {
    ridiculeTimer--;
    if (ridiculeTimer <= 0) {
      ridiculeTimer = RIDICULE_DURATION;
      ridiculed = false;
    }
  }

  if (glamoured) {
    glamourTimer--;
    if (glamourTimer <= 0) {
      glamourTimer = 1000;
      glamoured = false;
    }
  }

  if (blinded) {
    blindnessTimer--;
    if (blindnessTimer <= 0) {
      blindnessTimer = 700;
      blinded = false;
    }
  }

  if (paralyzed) {
    paralysisTimer--;
    if (paralysisTimer <= 0) {
      paralysisTimer = 1000;
      paralyzed = false;
    }
  }

  if (equippedArmor.item == ChaosArmor) {
    equippedArmorValue = random(0, 20 + equippedArmor.armorValue);
  }

  // If the player is close to a succubus, she slowly draws the player towards herself.
  if (!damsel[0].beingCarried && !succubusIsFriend) {
    for (int i = 0; i < maxEnemies; i++) {
      if (enemies[i].hp > 0 && strcmp(enemies[i].name, "succubus") == 0 && enemies[i].chasingPlayer) {
        float sdx = enemies[i].x - playerX;
        float sdy = enemies[i].y - playerY;
        float succubusDistanceSquared = sdx * sdx + sdy * sdy;
        if (damsel[0].levelOfLove < 6 || !damsel[0].followingPlayer || damsel[0].dead || !damsel[0].active) {
          if (succubusDistanceSquared < 40.0) {
            nearSuccubus = true;
            // While pulling, dialogue pops up
            snprintf(currentDialogue, sizeof(currentDialogue), "%s", "Hey there, handsome...");
            damsel[0].followingPlayer = false;
            currentDamselPortrait = succubusPortrait;
            showDialogue = true;
            if (dialogueTimeLength != 373) {
              playRawSFX3D(24, enemies[i].x, enemies[i].y);
            }
            dialogueTimeLength = 373;
            float pullStrength = 0.05; // Adjust this value to change pull strength
            playerX += (sdx / sqrt(succubusDistanceSquared)) * pullStrength;
            playerY += (sdy / sqrt(succubusDistanceSquared)) * pullStrength;
            // Ensure the player doesn't move into walls due to the pull
            int rPullX = round(playerX);
            int rPullY = round(playerY);
            if (dungeonMap[rPullY][rPullX] != Floor && dungeonMap[rPullY][rPullX] != Exit && dungeonMap[rPullY][rPullX] != StartStairs && dungeonMap[rPullY][rPullX] != DoorOpen) {
              playerX -= (sdx / sqrt(succubusDistanceSquared)) * pullStrength;
              playerY -= (sdy / sqrt(succubusDistanceSquared)) * pullStrength;
            }
          } else {
            nearSuccubus = false;
          }
        } else if (damsel[0].levelOfLove >= 6 && damsel[0].followingPlayer && !damsel[0].dead && damsel[0].active) {
          nearSuccubus = false;
          damsel[0].beingCarried = true;
          currentDamselPortrait = damselPortraitCarrying;
          dialogueTimeLength = 400;
          //currentDialogue = "You get away from him! He's mine!";
          snprintf(currentDialogue, sizeof(currentDialogue), "%s", "Stay away from her- you're mine!");
          showDialogue = true;
          playRawSFX(21);
          playerSprite = playerSprite == playerSpriteRight ? playerCarryingDamselSpriteRight : playerCarryingDamselSpriteLeft;
        }
      }
    }
  }
}

void playDamselSFX(const char *tone) {
  if (strcmp(tone, "normal") == 0) {
    playRawSFX3D(16, damsel[0].x, damsel[0].y);
  } else if (strcmp(tone, "annoying") == 0) {
    playRawSFX3D(21, damsel[0].x, damsel[0].y);
  } else if (strcmp(tone, "alone") == 0) {
    playRawSFX3D(16, damsel[0].x, damsel[0].y);
  }
}

int dialogueTimer = 0;
void handleDialogue() {
  static int dialogueDelayTimer = 0;
  
  // Glamour dialogue system
  if (glamoured) {
    static int lastGlamourIndex = -1;
    if (!showDialogue && dialogueDelayTimer <= 0) {
      int length = sizeof(glamourDialogue) / sizeof(glamourDialogue[0]);
      if (length <= 0) return; // Defensive: don't proceed if array is empty
      int index = random(0, length);
      if (index == lastGlamourIndex && length > 1) {
        index = (index + 1) % length;
      }
      lastGlamourIndex = index;
      dialogueTimeLength = glamourDialogue[index].duration;
      snprintf(currentDialogue, sizeof(currentDialogue), "%s", glamourDialogue[index].message);
      showDialogue = true;
      isRidiculeDialogue = true; // Reuse this to hide portrait
      dialogueDelayTimer = 50; // Add delay before next dialogue
    }
  }

  // Ridicule dialogue system
  if (ridiculed) {
    if (!showDialogue && dialogueDelayTimer <= 0) {
      int length = sizeof(ridiculeDialogue) / sizeof(ridiculeDialogue[0]);
      if (length <= 0) return; // Defensive: don't proceed if array is empty
      int index = random(0, length);
      if (index == lastRidiculeIndex && length > 1) {
        index = (index + 1) % length;
      }
      lastRidiculeIndex = index;
      dialogueTimeLength = ridiculeDialogue[index].duration;
      snprintf(currentDialogue, sizeof(currentDialogue), "%s", ridiculeDialogue[index].message);
      showDialogue = true;
      isRidiculeDialogue = true;
      dialogueDelayTimer = 50; // Add delay before next dialogue
    }
  }
  
  // Update dialogue delay timer
  if (dialogueDelayTimer > 0) {
    dialogueDelayTimer--;
  }

  // Drawing code (always runs if showDialogue is true)
  if (showDialogue) {
    dialogueTimer++;
    if (dialogueTimer >= dialogueTimeLength) {
      dialogueTimer = 0;
      if (dialogueTimeLength == 373) {
        dialogueTimeLength = 0;
      }
      showDialogue = false;
    }
    u8g2_for_adafruit_gfx.setFont(u8g2_font_profont10_mf);
    display.fillRect(25, 10, 100, 34, 0);
    u8g2_for_adafruit_gfx.setCursor(27, 19);
    drawWrappedText(27, 19, 96, currentDialogue);
    display.drawRect(25, 10, 100, 34, 15);
    if (!isRidiculeDialogue || strcmp(currentDialogue, "Hey! Wait up!") == 0) {
      display.drawBitmap(9, 11, currentDamselPortrait, 16, 32, 15);
      display.drawRect(8, 10, 18, 34, 15);
    }
  }

  if (damsel[0].followingPlayer && !damsel[0].dead && !succubusIsFriend) {
    timeTillNextDialogue--;
    if (timeTillNextDialogue <= 0) {
      // Helper lambda to pick a random unsaid dialogue from a given set.
      auto pickDialogue = [](Dialogue dialogueSet[], int length) -> int {
        int unsaidCount = 0;
        int unsaidIndices[length];
        for (int i = 0; i < length; i++) {
          if (!dialogueSet[i].alreadyBeenSaid) {
            unsaidIndices[unsaidCount++] = i;
          }
        }
        // If all dialogues have been used, reset them.
        //if (unsaidCount == 0) {
        //  for (int i = 0; i < length; i++) {
        //    dialogueSet[i].alreadyBeenSaid = false;
        //    unsaidIndices[i] = i;
        //  }
        //  unsaidCount = length;
        //}
        // Choose a random index from the unsaid list.
        int chosenIndex = unsaidIndices[random(0, unsaidCount)];
        return chosenIndex;
      };

      // Carrying damsel dialogue branch.
      if (!damsel[0].completelyRescued) {
        if (damsel[0].beingCarried) {
          int length = sizeof(damselCarryDialogue) / sizeof(damselCarryDialogue[0]);
          int index = pickDialogue(damselCarryDialogue, length);
          currentDamselPortrait = damselPortraitCarrying;
          dialogueTimeLength = damselCarryDialogue[index].duration;
          isRidiculeDialogue = false;
          snprintf(currentDialogue, sizeof(currentDialogue), "%s", damselCarryDialogue[index].message);
          if (!damselCarryDialogue[index].alreadyBeenSaid) {
            playRawSFX(18);
          }
          damselCarryDialogue[index].alreadyBeenSaid = true;
        } else {
          // Choose dialogue based on levelOfLove.
          if (damsel[0].levelOfLove >= 1 && damsel[0].levelOfLove < 3) {
            int length = sizeof(damselAnnoyingDialogue) / sizeof(damselAnnoyingDialogue[0]);
            int index = pickDialogue(damselAnnoyingDialogue, length);
            if (!damselAnnoyingDialogue[index].alreadyBeenSaid) {
              playDamselSFX(damselAnnoyingDialogue[index].tone);
            }
            currentDamselPortrait = (strcmp(damselAnnoyingDialogue[index].tone, "annoying") == 0) ? 
                                    damselPortraitScared : 
                                    (strcmp(damselAnnoyingDialogue[index].tone, "alone") == 0) ? 
                                    damselPortraitAlone : 
                                    damselPortraitNormal;
            dialogueTimeLength = damselAnnoyingDialogue[index].duration;
            isRidiculeDialogue = false;
            snprintf(currentDialogue, sizeof(currentDialogue), "%s", damselAnnoyingDialogue[index].message);
            damselAnnoyingDialogue[index].alreadyBeenSaid = true;
          } else if (damsel[0].levelOfLove >= 3 && damsel[0].levelOfLove < 6) {
            int length = sizeof(damselPassiveDialogue) / sizeof(damselPassiveDialogue[0]);
            int index = pickDialogue(damselPassiveDialogue, length);
            if (!damselPassiveDialogue[index].alreadyBeenSaid) {
              playDamselSFX(damselPassiveDialogue[index].tone);
            }
            currentDamselPortrait = (strcmp(damselPassiveDialogue[index].tone, "annoying") == 0) ? 
                                    damselPortraitScared : 
                                    (strcmp(damselPassiveDialogue[index].tone, "alone") == 0) ? 
                                    damselPortraitAlone : 
                                    damselPortraitNormal;
            dialogueTimeLength = damselPassiveDialogue[index].duration;
            isRidiculeDialogue = false;
            snprintf(currentDialogue, sizeof(currentDialogue), "%s", damselPassiveDialogue[index].message);
            damselPassiveDialogue[index].alreadyBeenSaid = true;
          } else if (damsel[0].levelOfLove >= 6) {
            if (!knowsDamselName) {
              dialogueTimeLength = 500;
              if (!knowsDamselName) {
                playDamselSFX("normal");
              }
              snprintf(currentDialogue, sizeof(currentDialogue), "By the way, my name is %s...", damsel[0].name);
              knowsDamselName = true;
            } else {
              int length = sizeof(damselGoodDialogue) / sizeof(damselGoodDialogue[0]);
              int index = pickDialogue(damselGoodDialogue, length);
              if (!damselGoodDialogue[index].alreadyBeenSaid) {
                playDamselSFX(damselGoodDialogue[index].tone);
              }
              currentDamselPortrait = (strcmp(damselGoodDialogue[index].tone, "annoying") == 0) ? 
                                      damselPortraitScared : 
                                      (strcmp(damselGoodDialogue[index].tone, "alone") == 0) ? 
                                      damselPortraitAlone : 
                                      damselPortraitNormal;
              dialogueTimeLength = damselGoodDialogue[index].duration;
              isRidiculeDialogue = false;
              snprintf(currentDialogue, sizeof(currentDialogue), "%s", damselGoodDialogue[index].message);
              damselGoodDialogue[index].alreadyBeenSaid = true;
            }
          }
        }
      }
      showDialogue = true;

      timeTillNextDialogue = random(1000, 2000);
    }
  }
}

void handleRiddles() {
  // Generate the riddle only once per riddle screen
  if (!riddleGenerated) {
    generateRiddleUI();
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1327_WHITE);
  
  // Display the riddle prompt.
  display.setCursor(0, 10);
  display.print("Solve this riddle!");
  
  // Display the riddle text below the prompt.
  display.setCursor(0, 22);
  display.print(currentRiddle.riddle);

  // Display the four answer options.
  // We'll assume each option is displayed on its own line.
  for (int i = 0; i < 4; i++) {
    int optionY = 70 + (i * 12);  // Adjust spacing as needed
    if (i == selectedRiddleOption) {
      // Highlight the currently selected option (for example, using inverse colors)
      display.setTextColor(SSD1327_BLACK, SSD1327_WHITE);  // Inverted
    } else {
      display.setTextColor(SSD1327_WHITE, SSD1327_BLACK);
    }
    display.setCursor(10, optionY);
    display.print(currentRiddle.options[i]);
  }
  
  display.display();

  // --- Handle button scrolling and selection ---
  // (Assume updateButtonStates() is being called elsewhere in your main loop)
  if (buttons.upPressed && !buttons.upPressedPrev) {
    playRawSFX(8);
    selectedRiddleOption--;
    if (selectedRiddleOption < 0) selectedRiddleOption = 0;  // Prevent underflow
  }
  if (buttons.downPressed && !buttons.downPressedPrev) {
    playRawSFX(8);
    selectedRiddleOption++;
    if (selectedRiddleOption > 3) selectedRiddleOption = 3;  // Maximum index is 3
  }
  
  // If the B button is pressed, check the answer.
  if (buttons.bPressed && !buttons.bPressedPrev) {
    playRawSFX(7);
    if (selectedRiddleOption == currentRiddle.correctOption) {
      playRawSFX(6);
      display.setTextColor(SSD1327_WHITE, SSD1327_BLACK);
      if (playerHP > 0) {
        snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "Correct! You are rewarded.");
      } else {
        snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "Correct! You are revived.");
        playerHP = (int)(playerMaxHP / 2);
      }
      // Give three random items as a reward
      for (int i = 0; i < 3; i++) {
        int category = random(0, 5); // 0: potion, 1: scroll, 2: ring, 3: armor, 4: mushroom
        GameItem reward;
        if (category == 0) {
          reward = getItem(getRandomPotion(random(0, NUM_POTIONS), false));
          addToInventory(reward, false);
        } else if (category == 1) {
          reward = getItem(Scroll);
          addToInventory(reward, false);
        } else if (category == 2) {
          reward = getItem(Ring);
          addToInventory(reward, false); // or true if you want rings to be possibly cursed
        } else if (category == 3) {
          GameItems armorTypes[] = { LeatherArmor, IronArmor, MagicRobe, Cloak };
          reward = getItem(armorTypes[random(0, 4)]);
          addToInventory(reward, false);
        } else if (category == 4) {
          reward = getItem(Mushroom);
          addToInventory(reward, false);
        }
      }
    } else {
      display.setTextColor(SSD1327_WHITE, SSD1327_BLACK);
      playRawSFX(13);
      snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "Wrong answer! You suffer.");
      playerHP -= 10;
      checkIfDeadFrom("stupidity");
    }
    currentUIState = UI_ITEM_RESULT;
    // Reset riddle so a new one can be generated next time.
    riddleGenerated = false;
  }
}

void handleRingEffects() {
    static int regenCounter = 0;
    static int teleportCounter = 0;
    if (regenRingsNumber > 0) {
        regenCounter += regenRingsNumber;
        if (regenCounter >= 70) { // Regenerate every 50 ticks (adjust as needed)
            if (playerHP < playerMaxHP) playerHP++;
            regenCounter = 0;
        }
    } else {
        regenCounter = 0;
    }

    if (teleportRingsNumber > 0) {
        teleportCounter += teleportRingsNumber;
        if (teleportCounter >= 130) {
            playRawSFX(14);
            int newX, newY;
            do {
              newX = random(0, mapWidth);
              newY = random(0, mapHeight);
            } while (dungeonMap[newY][newX] != Floor);
            playerX = newX;
            playerY = newY;
            teleportCounter = 0;
        }
    } else {
        teleportCounter = 0;
    }
}

void OpenChest(int cy, int cx, int dx) {
  // Require solving a random puzzle before opening
  if (!launchRandomPuzzle()) {
    return; // Player did not solve puzzle, do not open chest
  }
  // Open the chest: remove chest and spawn loot in 3x3 area
  playRawSFX(3); // Play pickup sound
  dungeonMap[cy][cx] = Floor;
  for (int ldx = -1; ldx <= 1; ldx++) {
    for (int ldy = -1; ldy <= 1; ldy++) {
      int lx = cx + ldx;
      int ly = cy + ldy;
      if (lx >= 0 && lx < mapWidth && ly >= 0 && ly < mapHeight && dungeonMap[ly][lx] == Floor) {
        // Use rarity-based loot spawning - chests can contain items up to rarity 4
        // This allows for better loot from chests compared to random dungeon spawns
        if (random(0, 100) < 85) { // 85% chance to spawn loot in each valid tile
          dungeonMap[ly][lx] = getRandomLootTile(5); // Max rarity 5 for chest loot
        }
      }
    }
  }
  dx = 2; // break outer loop
}