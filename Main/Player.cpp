#include "Player.h"
#include "HelperFunctions.h"
#include "Inventory.h"
#include "Dungeon.h"
#include "Entities.h"

String deathCause = "";
String currentDialogue = "";
float playerX = 0;
float playerY = 0;
float currentSpeedMultiplier = 1;
int playerHP = 100;
int playerMaxHP = 100;
int speedTimer = 1000;
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
bool combiningTwoItems = false;
bool playerMoving = false;
bool starving = false;
bool seeAll = false;
bool showDialogue = false;
GameItem combiningItem1 = {};
GameItem combiningItem2 = {};

void renderPlayer() {
  float screenX = (playerX - offsetX) * tileSize;
  float screenY = (playerY - offsetY) * tileSize;

  // Ensure the player is within the viewport
  if (screenX >= 0 && screenX < SCREEN_WIDTH && screenY >= 0 && screenY < SCREEN_HEIGHT) {
    display.drawBitmap((screenX + tileSize / 2) - tileSize/2, (screenY + tileSize / 2) - tileSize/2, playerSprite, tileSize, tileSize, 15);
  }
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

  if (speeding) {
    speedTimer--;
    if (speedTimer <= 0) {
      speedTimer = 1000;
      speeding = false;
      currentSpeedMultiplier = 1;
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

  float diagSpeed = speed * 0.7071;

  if (buttons.upPressed && !buttons.leftPressed && !buttons.rightPressed) {
    playerDY = -1;
    playerDX = 0;
    newY -= speed; // Move up
  } else if (buttons.downPressed && !buttons.leftPressed && !buttons.rightPressed) {
    playerDY = 1;
    playerDX = 0;
    newY += speed; // Move down
  } else if (buttons.leftPressed && !buttons.upPressed && !buttons.downPressed) {
    playerDX = -1;
    playerDY = 0;
    playerSprite = carryingDamsel ? playerCarryingDamselSpriteLeft : playerSpriteLeft;
    newX -= speed; // Move left
  } else if (buttons.rightPressed && !buttons.upPressed && !buttons.downPressed) {
    playerDX = 1;
    playerDY = 0;
    playerSprite = carryingDamsel ? playerCarryingDamselSpriteRight : playerSpriteRight;
    newX += speed; // Move right
  } else if (buttons.upPressed && buttons.leftPressed) {
    playerDY = -1;
    playerDX = -1;
    playerSprite = carryingDamsel ? playerCarryingDamselSpriteLeft : playerSpriteLeft;
    newY -= diagSpeed; // Move up & left
    newX -= diagSpeed; // Move up & left
  } else if (buttons.upPressed && buttons.rightPressed) {
    playerDY = -1;
    playerDX = 1;
    playerSprite = carryingDamsel ? playerCarryingDamselSpriteRight : playerSpriteRight;
    newY -= diagSpeed; // Move up & right
    newX += diagSpeed; // Move up & left
  } else if (buttons.downPressed && buttons.leftPressed) {
    playerDX = -1;
    playerDY = 1;
    playerSprite = carryingDamsel ? playerCarryingDamselSpriteLeft : playerSpriteLeft;
    newX -= diagSpeed; // Move left & down
    newY += diagSpeed; // Move up & left
  } else if (buttons.downPressed && buttons.rightPressed) {
    playerDX = 1;
    playerDY = 1;
    playerSprite = carryingDamsel ? playerCarryingDamselSpriteRight : playerSpriteRight;
    newX += diagSpeed; // Move right & down
    newY += diagSpeed; // Move up & left
  }

  // Check if the player is moving
  playerMoving = buttons.upPressed || buttons.downPressed || buttons.leftPressed || buttons.rightPressed ? true : false;

  float dx = playerX - damsel[0].x;
  float dy = playerY - damsel[0].y;
  float distanceSquared = dx * dx + dy * dy;

  if (buttons.bPressed) {
    if (distanceSquared <= 0.3 && !damsel[0].dead && damsel[0].levelOfLove >= 6) {
      startCarryingDamsel();
    }
  } else {
    carryingDelay = 0;
  }

  if (buttons.bPressed) {
    if (!reloading && !carryingDamsel && distanceSquared > 0.3) {
      shootProjectile(playerDX, playerDY); // Shoot in current direction
      reloading = true;
    }
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
      setTile((int)playerX, (int)playerY, Exit);
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
  if (dungeonMap[rNewY][rNewX] == Floor || dungeonMap[rNewY][rNewX] == Exit || dungeonMap[rNewY][rNewX] == StartStairs) {
    playerX = newX;
    playerY = newY;

    // Update viewport offset if needed
    if (playerX - offsetX < 2 && offsetX > 0) offsetX -= scrollSpeed;
    if (playerX - offsetX > viewportWidth - 3 && offsetX < mapWidth - viewportWidth) offsetX += scrollSpeed;
    if (playerY - offsetY < 2 && offsetY > 0) offsetY -= scrollSpeed;
    if (playerY - offsetY > viewportHeight - 3 && offsetY < mapHeight - viewportHeight) offsetY += scrollSpeed;
  } else if (dungeonMap[rNewY][rNewX] == Potion) {
    if (addToInventory(getItem(getRandomPotion(random(8))))) {
      dungeonMap[rNewY][rNewX] = Floor;
    }
  } else if (dungeonMap[rNewY][rNewX] == Map) {
    hasMap = true;
    dungeonMap[rNewY][rNewX] = Floor;
  } else if (dungeonMap[rNewY][rNewX] == MushroomItem) {
    if (addToInventory(getItem(Mushroom))) {
      dungeonMap[rNewY][rNewX] = Floor;
    }
  }

  int rPx = round(playerX);
  int rPy = round(playerY);

  // Check if the player reached the exit
  if (dungeonMap[rPy][rPx] == Exit) {
    if (!damsel[0].dead && !damsel[0].followingPlayer && damsel[0].active) {
      levelOfDamselDeath = dungeon;
      damsel[0].active = false;
    }
    statusScreen = true;
  }
}

void startCarryingDamsel() {
  carryingDelay += 1;
  display.fillRect(0, 0, carryingDelay, 15, (int)(carryingDelay/8));
  display.display();
  if (carryingDelay >= SCREEN_WIDTH) {
    carryingDamsel = !carryingDamsel;

    if (carryingDamsel) {
      playerSprite = playerSprite == playerSpriteRight ? playerCarryingDamselSpriteRight : playerCarryingDamselSpriteLeft;
    } else {
      playerSprite = playerSprite == playerCarryingDamselSpriteRight ? playerSpriteRight : playerSpriteLeft;
    }

    carryingDelay = 0;
  }
}

void handlePauseScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(27, 40);
  display.print("PAUSED");
  display.setTextSize(1);
  display.setCursor(24, 65);
  display.print("Press [START]");
  display.display();
}

int hungerTick = 0;
void handleHunger() {
  if (playerMoving) {hungerTick++;}
  hungerTick += playerMoving ? 2 : 1;

  if (hungerTick >= (starving ? 200 : 700)) {
    if (starving) {
      playerHP -= 1;
    } else {
      playerFood -= 1;
    }
    hungerTick = 0;
    starving = playerFood <= 0 ? true : false;
  }

  if (playerHP <= 0) {
    deathCause = "hunger";
  }
}

int dialogueTimer = 0;
void handleDialogue() {
  if (showDialogue) {
    dialogueTimer++;
    if (dialogueTimer >= dialogueTimeLength) {
      dialogueTimer = 0;
      showDialogue = false;
    }
    u8g2_for_adafruit_gfx.setFont(u8g2_font_profont10_mf);
    display.fillRect(25, 10, 100, 34, 0);
    u8g2_for_adafruit_gfx.setCursor(27, 19);
    drawWrappedText(27, 19, 96, currentDialogue);
    display.drawRect(25, 10, 100, 34, 15);
    display.drawBitmap(9, 11, currentDamselPortrait, 16, 32, 15);
    display.drawRect(8, 10, 18, 34, 15);
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
        int chosenIndex = unsaidIndices[random(0, unsaidCount)];
        return chosenIndex;
      };

      // Carrying damsel dialogue branch.
      if (carryingDamsel) {
        int length = sizeof(damselCarryDialogue) / sizeof(damselCarryDialogue[0]);
        int index = pickDialogue(damselCarryDialogue, length);
        currentDamselPortrait = damselPortraitCarrying;
        dialogueTimeLength = damselCarryDialogue[index].duration;
        currentDialogue = damselCarryDialogue[index].message;
        damselCarryDialogue[index].alreadyBeenSaid = true;
      } else {
        // Choose dialogue based on levelOfLove.
        if (damsel[0].levelOfLove >= 1 && damsel[0].levelOfLove < 3) {
          int length = sizeof(damselAnnoyingDialogue) / sizeof(damselAnnoyingDialogue[0]);
          int index = pickDialogue(damselAnnoyingDialogue, length);
          currentDamselPortrait = (damselAnnoyingDialogue[index].tone == "annoying") ? 
                                  damselPortraitScared : 
                                  (damselAnnoyingDialogue[index].tone == "alone") ? 
                                  damselPortraitAlone : 
                                  damselPortraitNormal;
          dialogueTimeLength = damselAnnoyingDialogue[index].duration;
          currentDialogue = damselAnnoyingDialogue[index].message;
          damselAnnoyingDialogue[index].alreadyBeenSaid = true;
        } else if (damsel[0].levelOfLove >= 3 && damsel[0].levelOfLove < 6) {
          int length = sizeof(damselPassiveDialogue) / sizeof(damselPassiveDialogue[0]);
          int index = pickDialogue(damselPassiveDialogue, length);
          currentDamselPortrait = (damselPassiveDialogue[index].tone == "annoying") ? 
                                  damselPortraitScared : 
                                  (damselPassiveDialogue[index].tone == "alone") ? 
                                  damselPortraitAlone : 
                                  damselPortraitNormal;
          dialogueTimeLength = damselPassiveDialogue[index].duration;
          currentDialogue = damselPassiveDialogue[index].message;
          damselPassiveDialogue[index].alreadyBeenSaid = true;
        } else if (damsel[0].levelOfLove >= 6) {
          int length = sizeof(damselGoodDialogue) / sizeof(damselGoodDialogue[0]);
          int index = pickDialogue(damselGoodDialogue, length);
          currentDamselPortrait = (damselGoodDialogue[index].tone == "annoying") ? 
                                  damselPortraitScared : 
                                  (damselGoodDialogue[index].tone == "alone") ? 
                                  damselPortraitAlone : 
                                  damselPortraitNormal;
          dialogueTimeLength = damselGoodDialogue[index].duration;
          currentDialogue = damselGoodDialogue[index].message;
          damselGoodDialogue[index].alreadyBeenSaid = true;
        }
      }

      timeTillNextDialogue = random(1000, 2000);
    }
  }
}