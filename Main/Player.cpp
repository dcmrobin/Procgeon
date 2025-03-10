#include "Player.h"
#include "HelperFunctions.h"
#include "Inventory.h"
#include "Dungeon.h"

String deathCause = "";
float playerX = 0;
float playerY = 0;
int playerHP = 100;
int playerMaxHP = 100;
int speedTimer = 1000;
int dungeon = 1;
int kills = 0;
int playerDX;
int playerDY;
bool speeding = false;
bool hasMap = false;
bool paused = false;
bool carryingDamsel = false;
bool damselGotTaken = false;
bool combiningTwoItems = false;
bool playerMoving = false;
bool starving = false;
GameItem combiningItem1 = { Null, PotionCategory, "Null", 0, 0, 0, 0, 0, String(""), String(""), String("") };
GameItem combiningItem2 = { Null, PotionCategory, "Null", 0, 0, 0, 0, 0, String(""), String(""), String("") };
int ingredient1index = 0;
int playerFood = 100;

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
void handleInput() {
  float newX = playerX;
  float newY = playerY;

  float speed = speeding ? 0.2 : 0.1;

  if (speeding) {
    speedTimer--;
    if (speedTimer <= 0) {
      speedTimer = 1000;
      speeding = false;
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
    if (addToInventory(getItem(getRandomPotion(random(7))))) {
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