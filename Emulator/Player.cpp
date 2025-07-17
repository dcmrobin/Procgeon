#include "Player.h"
#include "HelperFunctions.h"
#include "Inventory.h"
#include "Dungeon.h"
#include "Entities.h"
#include "GameAudio.h"
#include "Item.h"
#include "Puzzles.h"

std::string deathCause = "";
std::string currentDialogue = "";
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
bool speeding = false;
bool hasMap = false;
bool paused = false;
bool carryingDamsel = false;
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
int equippedArmorValue = 0;  // Current armor value (damage reduction)
GameItem equippedArmor = {};  // Currently equipped armor item
bool equippedRiddleStone = false;
int playerAttackDamage = 10; // Player's attack damage, can be increased by enchant scroll
bool ringOfSwiftnessActive = false;
bool ringOfStrengthActive = false;
bool ringOfWeaknessActive = false;
bool ringOfHungerActive = false;
bool ringOfRegenActive = false;
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

void renderPlayer() {
  float screenX = (playerX - offsetX) * tileSize;
  float screenY = (playerY - offsetY) * tileSize;

  // Ensure the player is within the viewport
  if (screenX >= 0 && screenX < SCREEN_WIDTH && screenY >= 0 && screenY < SCREEN_HEIGHT) {
    display.drawBitmap((screenX + tileSize / 2) - tileSize/2, (screenY + tileSize / 2) - tileSize/2, playerSprite, tileSize, tileSize, 15);

    // --- Show 'carry [b]' prompt if player can carry damsel ---
    float dx = playerX - damsel[0].x;
    float dy = playerY - damsel[0].y;
    float distanceSquared = dx * dx + dy * dy;
    if (!carryingDamsel && !damsel[0].dead && damsel[0].levelOfLove >= 6 && distanceSquared <= 0.4) {
      display.setTextSize(1);
      display.setTextColor(15, 0);
      // Center the text under the player sprite (estimate 6px per char, 9 chars)
      int textWidth = 9 * 6;
      int textX = (screenX + tileSize / 2) - (textWidth / 2);
      int textY = (screenY + tileSize) + 2;
      display.setCursor(textX, textY);
      display.print("Carry [B]");
    }
    // --- Show 'Open Chest [B]' prompt if player is adjacent to a chest ---
    int rPx = round(playerX);
    int rPy = round(playerY);
    bool nearChest = false;
    for (int dx = -1; dx <= 1 && !nearChest; dx++) {
      for (int dy = -1; dy <= 1 && !nearChest; dy++) {
        int cx = rPx + dx;
        int cy = rPy + dy;
        if (cx >= 0 && cx < mapWidth && cy >= 0 && cy < mapHeight && dungeonMap[cy][cx] == ChestTile) {
          nearChest = true;
        }
      }
    }
    if (nearChest) {
      display.setTextSize(1);
      display.setTextColor(15, 0);
      int textWidth = 14 * 6; // "Open Chest [B]" is 14 chars
      int textX = (screenX + tileSize / 2) - (textWidth / 2);
      int textY = (screenY + tileSize) + 12;
      display.setCursor(textX, textY);
      display.print("Open Chest [B]");
    }
  }

  // Update viewport offset if needed
  if (playerX - offsetX < 2 && offsetX > 0) offsetX -= scrollSpeed;
  if (playerX - offsetX > viewportWidth - 3 && offsetX < mapWidth - viewportWidth) offsetX += scrollSpeed;
  if (playerY - offsetY < 2 && offsetY > 0) offsetY -= scrollSpeed;
  if (playerY - offsetY > viewportHeight - 3 && offsetY < mapHeight - viewportHeight) offsetY += scrollSpeed;
}

int shootDelay = 0;
bool reloading;
int damselHealDelay = 0;
int carryingDelay = 0;
float baseSpeed = 0.1;
void handleInput() {
  float newX = playerX;
  float newY = playerY;

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
      playerSprite = carryingDamsel ? playerCarryingDamselSpriteLeft : playerSpriteLeft;
      newX -= speed; // Move left
    }
    playerActed = true;
  } else if (rightPressed && !upPressed && !downPressed) {
    if (!paralyzed) {
      playerDX = 1;
      playerDY = 0;
      playerSprite = carryingDamsel ? playerCarryingDamselSpriteRight : playerSpriteRight;
      newX += speed; // Move right
    }
    playerActed = true;
  } else if (upPressed && leftPressed) {
    if (!paralyzed) {
      playerDY = -1;
      playerDX = -1;
      playerSprite = carryingDamsel ? playerCarryingDamselSpriteLeft : playerSpriteLeft;
      newY -= diagSpeed; // Move up & left
      newX -= diagSpeed; // Move up & left
    }
    playerActed = true;
  } else if (upPressed && rightPressed) {
    if (!paralyzed) {
      playerDY = -1;
      playerDX = 1;
      playerSprite = carryingDamsel ? playerCarryingDamselSpriteRight : playerSpriteRight;
      newY -= diagSpeed; // Move up & right
      newX += diagSpeed; // Move up & left
    }
    playerActed = true;
  } else if (downPressed && leftPressed) {
    if (!paralyzed) {
      playerDX = -1;
      playerDY = 1;
      playerSprite = carryingDamsel ? playerCarryingDamselSpriteLeft : playerSpriteLeft;
      newX -= diagSpeed; // Move left & down
      newY += diagSpeed; // Move up & left
    }
    playerActed = true;
  } else if (downPressed && rightPressed) {
    if (!paralyzed) {
      playerDX = 1;
      playerDY = 1;
      playerSprite = carryingDamsel ? playerCarryingDamselSpriteRight : playerSpriteRight;
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
    if (!reloading && !carryingDamsel && distanceSquared > 0.3) {
      shootProjectile(playerDX, playerDY); // Shoot in current direction
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

  /*if (Serial.available() > 0) {// for debug purposes
    char input = Serial.read();
    if (input == '7') {
      setTile((int)playerX, (int)playerY, Exit);
    } else if (input == '8') {
      moveDamselToPos(playerX, playerY);
      if (!damsel[0].active) {
        Serial.println("The damsel is not active.");
      }
    } else if (input == '6') {
      addToInventory(getItem(getRandomPotion(rand() % (NUM_POTIONS + 1), false)), false);
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
  }*/

  int rNewX = round(newX);
  int rNewY = round(newY);

  // Check collision with walls
  if (dungeonMap[rNewY][rNewX] == Floor || dungeonMap[rNewY][rNewX] == Exit || dungeonMap[rNewY][rNewX] == StartStairs) {
    playerX = newX;
    playerY = newY;
  } else if (dungeonMap[rNewY][rNewX] == Potion) {
    if (addToInventory(getItem(getRandomPotion(rand() % 7, true)), false)) {
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
    // Randomly choose an armor type
    GameItems armorTypes[] = { LeatherArmor, IronArmor, MagicRobe, Cloak };
    GameItems randomArmor = armorTypes[rand() % 5];
    
    if (addToInventory(getItem(randomArmor), true)) {
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

  // Check if the player reached the exit
  if (dungeonMap[rPy][rPx] == Exit) {
    playRawSFX(11);
    if (!damsel[0].dead && !damsel[0].followingPlayer && damsel[0].active) {
      levelOfDamselDeath = dungeon;
      damsel[0].active = false;
    }
    statusScreen = true;
  }

  // --- Open chest only if B is pressed and player is adjacent to a chest ---
  if (buttons.bPressed && !buttons.bPressedPrev) {
    for (int dx = -1; dx <= 1; dx++) {
      for (int dy = -1; dy <= 1; dy++) {
        int cx = rPx + dx;
        int cy = rPy + dy;
        if (cx >= 0 && cx < mapWidth && cy >= 0 && cy < mapHeight && dungeonMap[cy][cx] == ChestTile) {
          OpenChest(cy, cx, dy);
          break;
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
    carryingDamsel = !carryingDamsel;

    if (carryingDamsel) {
      showDialogue = true;
      currentDamselPortrait = damselPortraitCarrying;
      dialogueTimeLength = 300;
      currentDialogue = "Oh! Thanks...";
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
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(27, 40);
  display.setTextColor(15, 0);
  display.print("PAUSED");
  display.setTextSize(1);
  display.setCursor(24, 65);
  display.print("Press [START]");
  display.display();
}

int hungerTick = 0;
void handleHungerAndEffects() {
  hungerTick += playerMoving || carryingDamsel ? 2 : 1;

  if (hungerTick >= (starving ? 200 : 700)) {
    if (starving) {
      playerHP -= 3;
    } else {
      int hungerDrain = 1;
      if (ringOfHungerActive) hungerDrain += 4; // Increase drain if hunger ring is active
      playerFood -= hungerDrain;
    }
    hungerTick = 0;
    starving = playerFood <= 0 ? true : false;
  }

  if (playerHP <= 0) {
    playRawSFX(10);
    deathCause = "hunger";
  }

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

  if (carryingDamsel) {
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
}

void playDamselSFX(std::string tone) {
  if (tone == "normal") {
    playRawSFX(16);
  } else if (tone == "annoying") {
    playRawSFX(21);
  } else if (tone == "alone") {
    playRawSFX(16);
  }
}

int dialogueTimer = 0;
void handleDialogue() {
  // Glamour dialogue system
  if (glamoured) {
    static int lastGlamourIndex = -1;
    if (!showDialogue) {
      int length = sizeof(glamourDialogue) / sizeof(glamourDialogue[0]);
      if (length <= 0) return; // Defensive: don't proceed if array is empty
      int index = rand() % (length + 1);
      if (index == lastGlamourIndex && length > 1) {
        index = (index + 1) % length;
      }
      lastGlamourIndex = index;
      dialogueTimeLength = glamourDialogue[index].duration;
      currentDialogue = glamourDialogue[index].message;
      showDialogue = true;
      isRidiculeDialogue = true; // Reuse this to hide portrait
    }
  }

  // Ridicule dialogue system
  if (ridiculed) {
    if (!showDialogue) {
      int length = sizeof(ridiculeDialogue) / sizeof(ridiculeDialogue[0]);
      if (length <= 0) return; // Defensive: don't proceed if array is empty
      int index = rand() % (length + 1);
      if (index == lastRidiculeIndex && length > 1) {
        index = (index + 1) % length;
      }
      lastRidiculeIndex = index;
      dialogueTimeLength = ridiculeDialogue[index].duration;
      currentDialogue = ridiculeDialogue[index].message;
      showDialogue = true;
      isRidiculeDialogue = true;
    }
  }

  // Drawing code (always runs if showDialogue is true)
  if (showDialogue) {
    dialogueTimer++;
    if (dialogueTimer >= dialogueTimeLength) {
      dialogueTimer = 0;
      showDialogue = false;
    }
    //display.setFont(Adafruit_GFX::profont10_font);
    display.fillRect(25, 10, 100, 34, 0);
    drawWrappedText(27, 13, 120, currentDialogue);
    display.drawRect(25, 10, 100, 34, 15);
    if (!isRidiculeDialogue) {
      display.drawBitmap(9, 11, currentDamselPortrait, 16, 32, 15);
      display.drawRect(8, 10, 18, 34, 15);
    }
  }

  if (damsel[0].followingPlayer && !damsel[0].dead) {
    timeTillNextDialogue--;
    if (timeTillNextDialogue <= 0) {
      showDialogue = true;

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
        int chosenIndex = unsaidIndices[rand() % (unsaidCount + 1)];
        return chosenIndex;
      };

      // Carrying damsel dialogue branch.
      if (carryingDamsel) {
        int length = sizeof(damselCarryDialogue) / sizeof(damselCarryDialogue[0]);
        int index = pickDialogue(damselCarryDialogue, length);
        currentDamselPortrait = damselPortraitCarrying;
        dialogueTimeLength = damselCarryDialogue[index].duration;
        isRidiculeDialogue = false;
        currentDialogue = damselCarryDialogue[index].message;
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
          currentDamselPortrait = (damselAnnoyingDialogue[index].tone == "annoying") ? 
                                  damselPortraitScared : 
                                  (damselAnnoyingDialogue[index].tone == "alone") ? 
                                  damselPortraitAlone : 
                                  damselPortraitNormal;
          dialogueTimeLength = damselAnnoyingDialogue[index].duration;
          isRidiculeDialogue = false;
          currentDialogue = damselAnnoyingDialogue[index].message;
          damselAnnoyingDialogue[index].alreadyBeenSaid = true;
        } else if (damsel[0].levelOfLove >= 3 && damsel[0].levelOfLove < 6) {
          int length = sizeof(damselPassiveDialogue) / sizeof(damselPassiveDialogue[0]);
          int index = pickDialogue(damselPassiveDialogue, length);
          if (!damselPassiveDialogue[index].alreadyBeenSaid) {
            playDamselSFX(damselPassiveDialogue[index].tone);
          }
          currentDamselPortrait = (damselPassiveDialogue[index].tone == "annoying") ? 
                                  damselPortraitScared : 
                                  (damselPassiveDialogue[index].tone == "alone") ? 
                                  damselPortraitAlone : 
                                  damselPortraitNormal;
          dialogueTimeLength = damselPassiveDialogue[index].duration;
          isRidiculeDialogue = false;
          currentDialogue = damselPassiveDialogue[index].message;
          damselPassiveDialogue[index].alreadyBeenSaid = true;
        } else if (damsel[0].levelOfLove >= 6) {
          if (!knowsDamselName) {
            dialogueTimeLength = 500;
            if (!knowsDamselName) {
              playDamselSFX("normal");
            }
            currentDialogue = "By the way, my name is " + damsel[0].name + "...";
            knowsDamselName = true;
          } else {
            int length = sizeof(damselGoodDialogue) / sizeof(damselGoodDialogue[0]);
            int index = pickDialogue(damselGoodDialogue, length);
            if (!damselGoodDialogue[index].alreadyBeenSaid) {
              playDamselSFX(damselGoodDialogue[index].tone);
            }
            currentDamselPortrait = (damselGoodDialogue[index].tone == "annoying") ? 
                                    damselPortraitScared : 
                                    (damselGoodDialogue[index].tone == "alone") ? 
                                    damselPortraitAlone : 
                                    damselPortraitNormal;
            dialogueTimeLength = damselGoodDialogue[index].duration;
            isRidiculeDialogue = false;
            currentDialogue = damselGoodDialogue[index].message;
            damselGoodDialogue[index].alreadyBeenSaid = true;
          }
        }
      }

      timeTillNextDialogue = rand() % 1000 + 2000;
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
  display.setTextColor(SSD1327_WHITE, 0);
  
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
      itemResultMessage = "Correct! You are rewarded.";
      // Give three random items as a reward
      for (int i = 0; i < 3; i++) {
        int category = rand() % (5 + 1); // 0: potion, 1: scroll, 2: ring, 3: armor, 4: mushroom
        GameItem reward;
        if (category == 0) {
          reward = getItem(getRandomPotion(rand() % (NUM_POTIONS+1), false));
          addToInventory(reward, false);
        } else if (category == 1) {
          reward = getItem(Scroll);
          addToInventory(reward, false);
        } else if (category == 2) {
          reward = getItem(Ring);
          addToInventory(reward, false); // or true if you want rings to be possibly cursed
        } else if (category == 3) {
          GameItems armorTypes[] = { LeatherArmor, IronArmor, MagicRobe, Cloak };
          reward = getItem(armorTypes[rand() % (4 + 1)]);
          addToInventory(reward, false);
        } else if (category == 4) {
          reward = getItem(Mushroom);
          addToInventory(reward, false);
        }
      }
    } else {
      playRawSFX(13);
      itemResultMessage = "Wrong answer! You suffer.";
      playerHP -= 10;
      if (playerHP <= 0) {
        playRawSFX(10);
        deathCause = "stupidity";
      }
    }
    currentUIState = UI_ITEM_RESULT;
    // Reset riddle so a new one can be generated next time.
    riddleGenerated = false;
  }
}

void handleRingEffects() {
    static int regenCounter = 0;
    if (ringOfRegenActive) {
        regenCounter++;
        if (regenCounter >= 50) { // Regenerate every 50 ticks (adjust as needed)
            if (playerHP < playerMaxHP) playerHP++;
            regenCounter = 0;
        }
    } else {
        regenCounter = 0;
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
        int lootType = rand() % (5 + 1); // 0: potion, 1: scroll, 2: ring, 3: armor, 4: riddle stone
        if (lootType == 0 && rand() % 101 < 60) {
          dungeonMap[ly][lx] = RiddleStoneTile;
        } else if (lootType == 1 && rand() % 101 < 80) {
          dungeonMap[ly][lx] = ArmorTile;
        } else if (lootType == 2 && rand() % 101 < 80) {
          dungeonMap[ly][lx] = RingTile;
        } else if (lootType == 3 && rand() % 101 < 80) {
          dungeonMap[ly][lx] = ScrollTile;
        } else if (lootType == 4) {
          dungeonMap[ly][lx] = Potion;
        }
      }
    }
  }
  dx = 2; // break outer loop
}