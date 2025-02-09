#include "Player.h"
#include "HelperFunctions.h"
#include "Inventory.h"

float playerX = 0;
float playerY = 0;
int playerHP = 100;
int playerMaxHP = 100;
bool speeding = false;
int speedTimer = 1000;
bool hasMap = false;
String deathCause = "";
int dungeon = 1;
int kills = 0;
int playerDX;
int playerDY;

void renderPlayer() {
  float screenX = (playerX - offsetX) * tileSize;
  float screenY = (playerY - offsetY) * tileSize;

  // Ensure the player is within the viewport
  if (screenX >= 0 && screenX < SCREEN_WIDTH && screenY >= 0 && screenY < SCREEN_HEIGHT) {
    u8g2.drawXBMP((screenX + tileSize / 2) - tileSize/2, (screenY + tileSize / 2) - tileSize/2, tileSize, tileSize, playerSprite);
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
    newY -= speed; // Move up & left
    newX -= speed; // Move up & left
  } else if (buttons.upPressed && buttons.rightPressed) {
    playerDY = -1;
    playerDX = 1;
    playerSprite = playerSpriteRight;
    newY -= speed; // Move up & right
    newX += speed; // Move up & left
  } else if (buttons.downPressed && buttons.leftPressed) {
    playerDX = -1;
    playerDY = 1;
    playerSprite = playerSpriteLeft;
    newX -= speed; // Move left & down
    newY += speed; // Move up & left
  } else if (buttons.downPressed && buttons.rightPressed) {
    playerDX = 1;
    playerDY = 1;
    playerSprite = playerSpriteRight;
    newX += speed; // Move right & down
    newY += speed; // Move up & left
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
      setTile((int)playerX, (int)playerY, 4);
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
  if (dungeonMap[rNewY][rNewX] == 1 || dungeonMap[rNewY][rNewX] == 4 || dungeonMap[rNewY][rNewX] == 0) {
    playerX = newX;
    playerY = newY;

    // Update viewport offset if needed
    if (playerX - offsetX < 2 && offsetX > 0) offsetX -= scrollSpeed;
    if (playerX - offsetX > viewportWidth - 3 && offsetX < mapWidth - viewportWidth) offsetX += scrollSpeed;
    if (playerY - offsetY < 2 && offsetY > 0) offsetY -= scrollSpeed;
    if (playerY - offsetY > viewportHeight - 3 && offsetY < mapHeight - viewportHeight) offsetY += scrollSpeed;
  } else if (dungeonMap[rNewY][rNewX] == 5) {
    if (addToInventory(getItem(getRandomPotion(random(7))))) {
      dungeonMap[rNewY][rNewX] = 1;
    }
  } else if (dungeonMap[rNewY][rNewX] == 6) {
    hasMap = true;
    dungeonMap[rNewY][rNewX] = 1;
  }

  int rPx = round(playerX);
  int rPy = round(playerY);

  // Check if the player reached the exit
  if (dungeonMap[rPy][rPx] == 4) {
    Serial.println("You reached the exit!");
    if (!damsel[0].dead && !damsel[0].followingPlayer && damsel[0].active) {
      levelOfDamselDeath = dungeon;
      damsel[0].active = false;
    }
    statusScreen = true;
  }
}