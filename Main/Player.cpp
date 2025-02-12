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
    playerSprite = playerSpriteLeft;
    newX -= speed; // Move left
  } else if (buttons.rightPressed && !buttons.upPressed && !buttons.downPressed) {
    playerDX = 1;
    playerDY = 0;
    playerSprite = playerSpriteRight;
    newX += speed; // Move right
  } else if (buttons.upPressed && buttons.leftPressed) {
    playerDY = -1;
    playerDX = -1;
    playerSprite = playerSpriteLeft;
    newY -= diagSpeed; // Move up & left
    newX -= diagSpeed; // Move up & left
  } else if (buttons.upPressed && buttons.rightPressed) {
    playerDY = -1;
    playerDX = 1;
    playerSprite = playerSpriteRight;
    newY -= diagSpeed; // Move up & right
    newX += diagSpeed; // Move up & left
  } else if (buttons.downPressed && buttons.leftPressed) {
    playerDX = -1;
    playerDY = 1;
    playerSprite = playerSpriteLeft;
    newX -= diagSpeed; // Move left & down
    newY += diagSpeed; // Move up & left
  } else if (buttons.downPressed && buttons.rightPressed) {
    playerDX = 1;
    playerDY = 1;
    playerSprite = playerSpriteRight;
    newX += diagSpeed; // Move right & down
    newY += diagSpeed; // Move up & left
  }

  if (buttons.bPressed && !reloading) {
    shootProjectile(playerDX, playerDY); // Shoot in current direction
    reloading = true;
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