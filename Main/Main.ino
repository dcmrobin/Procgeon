#include <U8g2lib.h>
#include <EEPROM.h>
#include "Sprites.h"
#include "Dungeon.h"
#include "HelperFunctions.h"
#include "Entities.h"

// OLED display pins
#define OLED_MOSI 11
#define OLED_CLK 13
#define OLED_DC 7
#define OLED_CS 10
#define OLED_RST 9
#define BUTTON_UP_PIN    2
#define BUTTON_DOWN_PIN  3
#define BUTTON_LEFT_PIN  4
#define BUTTON_RIGHT_PIN 5
#define BUTTON_B_PIN 1
#define BUTTON_A_PIN 0

// Viewport size (in tiles)
const int viewportWidth = 128 / tileSize;
const int viewportHeight = 128 / tileSize - 2;

// Map scrolling offset
float offsetX = 0;
float offsetY = 0;

// Smooth scrolling speed
const float scrollSpeed = 0.25f;

// Player position
float playerX;
float playerY;

// Player stats
int playerHP = 100;
int level = 1;
int kills = 0;
bool statusScreen = false;
unsigned int lvlHighscoreAddress = 0;
unsigned int killHighscoreAddress = 1;
const char* deathCause = "";

struct Projectile {
  float x, y;
  float dx, dy;
  float speed;
  float damage;
  bool active;
};
const int maxProjectiles = 30;
Projectile projectiles[maxProjectiles];

int playerDX;
int playerDY;

// SH1107 128x128 SPI Constructor
U8G2_SH1107_PIMORONI_128X128_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RST);

// Timing variables
unsigned long lastUpdateTime = 0;
const unsigned long frameDelay = 20; // Update every 100ms

void setup() {
  Serial.begin(9600);
  u8g2.begin();
  u8g2.setBitmapMode(1);

  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);

  for (int i = 0; i < maxProjectiles; i++) {
      projectiles[i].active = false;
  }

  randomSeed(generateRandomSeed());

  // Generate a random dungeon
  generateDungeon(playerX, playerY, damsel[0]);
  spawnEnemies();
}

void loop() {
  unsigned long currentTime = millis();
  if (playerHP > 0) {
    if (currentTime - lastUpdateTime >= frameDelay) {
      lastUpdateTime = currentTime;

      if (!statusScreen) {
        // Update game state
        handleInput();
        updateScrolling();
        updateDamsel();
        updateEnemies();
        updateProjectiles();

        // Render the game
        u8g2.clearBuffer();
        renderDungeon();
        renderDamsel();
        renderEnemies();
        renderProjectiles();
        renderPlayer();
        updateAnimations();
        renderUI();
        u8g2.sendBuffer();
      } else {
        showStatusScreen();
      }
    }
  }
  else {
    gameOver();
  }
}

int blobanimcounter = 0;
int damselanimcounter = 0;
void updateAnimations() {
  blobanimcounter += 1;
  if (blobanimcounter >= 20) {
    blobSprite = blobSprite == blobSpriteFrame1 ? blobSpriteFrame2 : blobSpriteFrame1;
    blobanimcounter = 0;
  }
  
  damselanimcounter += 1;
  if (damselanimcounter >= random(50, 90)) {
    damselSprite = damsel[0].dead ? damselSpriteDead : damselSprite;
    damselanimcounter = 0;
  }
}

void updateScrolling() {
  // Target offsets based on player's position
  float targetOffsetX = playerX - (viewportWidth / 2.0f) + 0.5f;
  float targetOffsetY = playerY - (viewportHeight / 2.0f) + 0.5f;

  // Clamp target offsets to map boundaries
  targetOffsetX = constrain(targetOffsetX, 0, mapWidth - viewportWidth);
  targetOffsetY = constrain(targetOffsetY, 0, mapHeight - viewportHeight);

  // Smoothly move the offset towards the target
  offsetX += (targetOffsetX - offsetX) * scrollSpeed;
  offsetY += (targetOffsetY - offsetY) * scrollSpeed;
}

// Render the visible portion of the dungeon
void renderDungeon() {
  for (int y = 0; y < viewportHeight + 1; y++) { // +1 to handle partial tiles at edges
    for (int x = 0; x < viewportWidth + 1; x++) {
      float mapX = x + offsetX;
      float mapY = y + offsetY;

      if (mapX >= 0 && mapX < mapWidth && mapY >= 0 && mapY < mapHeight) {
        // Calculate screen position based on fractional offsets
        float screenX = (x - (offsetX - (int)offsetX)) * tileSize;
        float screenY = (y - (offsetY - (int)offsetY)) * tileSize;

        drawTile((int)mapX, (int)mapY, screenX, screenY);
      }
    }
  }
}

// Draw a tile based on its type
void drawTile(int mapX, int mapY, float screenX, float screenY) {
  int tileType = dungeonMap[mapY][mapX];

  switch (tileType) {
    case 0: // Start stairs
      u8g2.drawXBMP(screenX, screenY, tileSize, tileSize, stairsSprite);
      break;
    case 1: // Floor
      //u8g2.drawFrame(screenX, screenY, tileSize, tileSize);
      break;
    case 2: // Wall
      //u8g2.drawBox(screenX, screenY, tileSize, tileSize);
      u8g2.drawXBMP(screenX, screenY, tileSize, tileSize, wallSprite);
      break;
    case 3: // Bars
      u8g2.drawXBMP(screenX, screenY, tileSize, tileSize, barsSprite);
      break;
    case 4: // Exit
      u8g2.drawXBMP(screenX, screenY, tileSize, tileSize, stairsSprite);
      break;
  }
}

// Render the player
void renderPlayer() {
  float screenX = (playerX - offsetX) * tileSize;
  float screenY = (playerY - offsetY) * tileSize;

  // Ensure the player is within the viewport
  if (screenX >= 0 && screenX < 128 && screenY >= 0 && screenY < 128) {
    //u8g2.drawDisc(screenX + tileSize / 2, screenY + tileSize / 2, tileSize / 3, U8G2_DRAW_ALL);
    u8g2.drawXBMP((screenX + tileSize / 2) - tileSize/2, (screenY + tileSize / 2) - tileSize/2, tileSize, tileSize, playerSprite);
  }
}

void spawnEnemies() {
  for (int i = 0; i < maxEnemies; i++) {
    while (true) {
      int ex = random(0, mapWidth);
      int ey = random(0, mapHeight);
      if (dungeonMap[ey][ex] == 1) { // Only spawn on floor tiles
        enemies[i] = {(float)ex, (float)ey, 20, false, 0.05, "blob", 20};
        break;
      }
    }
  }
}

int atkDelayCounter = 0;
void updateEnemies() {
  atkDelayCounter += 1;
  for (int i = 0; i < maxEnemies; i++) {
    if (enemies[i].hp <= 0) continue; // Skip dead enemies

    // Calculate distance to player
    int dx = round(playerX) - round(enemies[i].x);
    int dy = round(playerY) - round(enemies[i].y);
    int distanceSquared = dx * dx + dy * dy;

    // Check if enemy should chase the player
    if (distanceSquared <= 25) { // Chase if within 5 tiles (distance^2 = 25)
      enemies[i].chasingPlayer = true;
    } else {
      enemies[i].chasingPlayer = false;
    }

    if (enemies[i].chasingPlayer) {
      // Determine the primary direction of movement
      float moveX = (dx > 0 ? 1 : dx < 0 ? -1 : 0);
      float moveY = (dy > 0 ? 1 : dy < 0 ? -1 : 0);

      // Normalize movement vector
      float magnitude = sqrt(moveX * moveX + moveY * moveY);
      if (magnitude > 0) {
        moveX /= magnitude;
        moveY /= magnitude;
      }

      // Attempt to move diagonally first
      float nx = enemies[i].x + moveX * enemies[i].moveAmount;
      float ny = enemies[i].y + moveY * enemies[i].moveAmount;

      bool xValid = !checkSpriteCollisionWithTileX(nx, enemies[i].x, enemies[i].y);
      bool yValid = !checkSpriteCollisionWithTileY(ny, enemies[i].y, enemies[i].x);

      if (xValid && yValid) {
        // Move diagonally if both directions are valid
        enemies[i].x = nx;
        enemies[i].y = ny;
      } else if (xValid) {
        // Slide along X if Y is blocked
        enemies[i].x = nx;
      } else if (yValid) {
        // Slide along Y if X is blocked
        enemies[i].y = ny;
      } else {
        // Both directions blocked, try "wall sliding"
        // Check perpendicular sliding directions
        float slideX = enemies[i].x + moveX * enemies[i].moveAmount;
        float slideY = enemies[i].y;

        if (!checkSpriteCollisionWithTileX(slideX, enemies[i].x, enemies[i].y)) {
          enemies[i].x = slideX;
        } else if (!checkSpriteCollisionWithTileY(slideY, enemies[i].y, enemies[i].x)) {
          enemies[i].y = slideY;
        }
      }
    } else {
      // Random wandering if not chasing
      int dir = random(0, 4);
      float nx = enemies[i].x + (dir == 0 ? enemies[i].moveAmount * 2 : dir == 1 ? -enemies[i].moveAmount * 2 : 0);
      float ny = enemies[i].y + (dir == 2 ? enemies[i].moveAmount * 2 : dir == 3 ? -enemies[i].moveAmount * 2 : 0);

      if (!checkSpriteCollisionWithTileX(nx, enemies[i].x, enemies[i].y)) {
        enemies[i].x = nx;
      }
      if (!checkSpriteCollisionWithTileY(ny, enemies[i].y, enemies[i].x)) {
        enemies[i].y = ny;
      }
    }

    // Check for collision with the player
    if (checkSpriteCollisionWithSprite(enemies[i].x, enemies[i].y, playerX, playerY)) {
      if (atkDelayCounter >= enemies[i].attackDelay) {
        playerHP -= 5; // Damage player
        atkDelayCounter = 0;
      }
      if (playerHP <= 0) {
        deathCause = enemies[i].name;
      }
    }
  }
}

void renderEnemies() {
  for (int i = 0; i < maxEnemies; i++) {
    if (enemies[i].hp > 0) {
      float screenX = (enemies[i].x - offsetX) * tileSize;
      float screenY = (enemies[i].y - offsetY) * tileSize;
      if (screenX >= 0 && screenY >= 0 && screenX < 128 && screenY < 128) {
        u8g2.drawXBMP(screenX, screenY, 8, 8, blobSprite);
      }
    }
  }
}

int damselMoveDelay = 0;
void updateDamsel() {
  if (!damsel[0].dead) {
    damselMoveDelay++;
  }

  float destinationX;
  float destinationY;

  destinationX = playerDX == 1 ? playerX - 1 : playerDX == -1 ? playerX + 1 : playerX;
  destinationY = playerDY == 1 ? playerY - 1 : playerDY == -1 ? playerY + 1 : playerY;

  // Calculate distance to player
  int dx = round(destinationX) - round(damsel[0].x);
  int dy = round(destinationY) - round(damsel[0].y);
  int distanceSquared = dx * dx + dy * dy;

  // Check if the damsel should follow the player
  if (distanceSquared <= 25) { // Follow if within 5 tiles (distance^2 = 25)
    damsel[0].followingPlayer = true;
    damsel[0].speed = 0.3;
  } else {
    damsel[0].followingPlayer = false;
    damsel[0].speed = 0.1;
  }

  if (!damsel[0].followingPlayer) {
    // Random wandering
    if (damselMoveDelay >= 30) {
      int dir = random(0, 4);
      float nx = damsel[0].x + (dir == 0 ? damsel[0].speed : dir == 1 ? -damsel[0].speed : 0);
      float ny = damsel[0].y + (dir == 2 ? damsel[0].speed : dir == 3 ? -damsel[0].speed : 0);

      damselSprite = dir == 0 ? damselSpriteRight : dir == 1 ? damselSpriteLeft : damselSprite;

      // Check bounds and avoid walls
      if (!checkSpriteCollisionWithTileX(nx, damsel[0].x, ny)) {
        damsel[0].x = nx;
      }
      if (!checkSpriteCollisionWithTileY(ny, damsel[0].y, nx)) {
        damsel[0].y = ny;
      }
      damselMoveDelay = 0;
    }
  } else {
    // Following the player
    if (damselMoveDelay >= 3) {
      float moveX = (dx > 0 ? 1 : dx < 0 ? -1 : 0);
      float moveY = (dy > 0 ? 1 : dy < 0 ? -1 : 0);

      damselSprite = moveX == 1 ? damselHopefullSpriteRight : moveX == -1 ? damselHopefullSpriteLeft : damselSprite;

      // Normalize movement vector
      float magnitude = sqrt(moveX * moveX + moveY * moveY);
      if (magnitude > 0) {
        moveX /= magnitude;
        moveY /= magnitude;
      }

      // Attempt to move diagonally first
      float nx = damsel[0].x + moveX * damsel[0].speed;
      float ny = damsel[0].y + moveY * damsel[0].speed;

      bool xValid = !checkSpriteCollisionWithTileX(nx, damsel[0].x, damsel[0].y);
      bool yValid = !checkSpriteCollisionWithTileY(ny, damsel[0].y, damsel[0].x);

      if (xValid && yValid) {
        // Move diagonally
        damsel[0].x = nx;
        damsel[0].y = ny;
      } else if (xValid) {
        // Slide along X
        damsel[0].x = nx;
      } else if (yValid) {
        // Slide along Y
        damsel[0].y = ny;
      } else {
        // Both directions blocked, try wall sliding
        float slideX = damsel[0].x + moveX * damsel[0].speed;
        float slideY = damsel[0].y;

        if (!checkSpriteCollisionWithTileX(slideX, damsel[0].x, damsel[0].y)) {
          damsel[0].x = slideX;
        } else if (!checkSpriteCollisionWithTileY(slideY, damsel[0].y, damsel[0].x)) {
          damsel[0].y = slideY;
        }
      }

      damselMoveDelay = 0;
    }
  }
}

void renderDamsel() {
  float screenX = (damsel[0].x - offsetX) * tileSize;
  float screenY = (damsel[0].y - offsetY) * tileSize;
  if (screenX >= 0 && screenY >= 0 && screenX < 128 && screenY < 128) {
    u8g2.drawXBMP(screenX, screenY, 8, 8, damselSprite);
  }
}

void shootProjectile(float xDir, float yDir) {

  for (int i = 0; i < maxProjectiles; i++) {
      if (!projectiles[i].active) {
          projectiles[i].x = playerX;
          projectiles[i].y = playerY;
          projectiles[i].dx = xDir;  // Set direction based on player's facing direction
          projectiles[i].dy = yDir;
          projectiles[i].damage = 10;
          projectiles[i].speed = 0.5;
          projectiles[i].active = true;
          break;
      }
  }
}

void updateProjectiles() {
  for (int i = 0; i < maxProjectiles; i++) {
    if (projectiles[i].active) {
      projectiles[i].x += projectiles[i].dx * projectiles[i].speed;
      projectiles[i].y += projectiles[i].dy * projectiles[i].speed;

      int projectileTileX = predictXtile(projectiles[i].x);
      int projectileTileY = predictYtile(projectiles[i].y);

      // Check for collisions with walls or out-of-bounds
      if (dungeonMap[projectileTileY][projectileTileX] != 1 || projectiles[i].x < 0 || projectiles[i].y < 0 || projectiles[i].x > 128 || projectiles[i].y > 128) {
          projectiles[i].active = false; // Deactivate the bullet
          //free(projectiles[i]);
      }

      // Check for collisions with enemies
      for (int j = 0; j < maxEnemies; j++) {
        if (checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y, enemies[j].x, enemies[j].y) && enemies[j].hp > 0) {
          enemies[j].hp -= projectiles[i].damage;    // Reduce enemy health
          if (enemies[j].hp <= 0 && projectiles[i].active == true) {
            kills += 1;
          }
          projectiles[i].active = false; // Deactivate the bullet
        } else if (!damsel[0].dead && checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y, damsel[0].x, damsel[0].y)) {
          damsel[0].dead = true;
          projectiles[i].active = false;
        }
      }
    }
  }
}

void renderProjectiles() {
    for (int i = 0; i < maxProjectiles; i++) {
        if (projectiles[i].active) {
          float screenX = (projectiles[i].x - offsetX) * tileSize + tileSize/2;
          float screenY = (projectiles[i].y - offsetY) * tileSize + tileSize/2;
          u8g2.drawDisc(screenX, screenY, 1);
        }
    }
}

// Render the UI
void renderUI() { 
  char HP[4];
  char Lvl[7];
  snprintf(HP, sizeof(HP), "%d", playerHP); // Convert playerHP to a string
  snprintf(Lvl, sizeof(Lvl), "%d", level);
  
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(5, 123, "HP:");
  u8g2.drawStr(20, 123, HP);
  u8g2.drawStr(40, 123, "LVL:");
  u8g2.drawStr(60, 123, Lvl);
  u8g2.drawFrame(0, 113, 128, 15);
}

int shootDelay = 0;
bool reloading;
void handleInput() {
  float newX = playerX;
  float newY = playerY;

  // Read button states (inverted because of pull-up resistors)
  bool upPressed = !digitalRead(BUTTON_UP_PIN);
  bool downPressed = !digitalRead(BUTTON_DOWN_PIN);
  bool leftPressed = !digitalRead(BUTTON_LEFT_PIN);
  bool rightPressed = !digitalRead(BUTTON_RIGHT_PIN);
  bool bPressed = !digitalRead(BUTTON_B_PIN);
  //bool aPressed = !digitalRead(BUTTON_A_PIN);

  if (upPressed && !leftPressed && !rightPressed) {
    playerDY = -1;
    playerDX = 0;
    newY -= 0.1; // Move up
  } else if (downPressed && !leftPressed && !rightPressed) {
    playerDY = 1;
    playerDX = 0;
    newY += 0.1; // Move down
  } else if (leftPressed && !upPressed && !downPressed) {
    playerDX = -1;
    playerDY = 0;
    playerSprite = playerSpriteLeft;
    newX -= 0.1; // Move left
  } else if (rightPressed && !upPressed && !downPressed) {
    playerDX = 1;
    playerDY = 0;
    playerSprite = playerSpriteRight;
    newX += 0.1; // Move right
  } else if (upPressed && leftPressed) {
    playerDY = -1;
    playerDX = -1;
    playerSprite = playerSpriteLeft;
    newY -= 0.1; // Move up & left
    newX -= 0.1; // Move up & left
  } else if (upPressed && rightPressed) {
    playerDY = -1;
    playerDX = 1;
    playerSprite = playerSpriteRight;
    newY -= 0.1; // Move up & right
    newX += 0.1; // Move up & left
  } else if (downPressed && leftPressed) {
    playerDX = -1;
    playerDY = 1;
    playerSprite = playerSpriteLeft;
    newX -= 0.1; // Move left & down
    newY += 0.1; // Move up & left
  } else if (downPressed && rightPressed) {
    playerDX = 1;
    playerDY = 1;
    playerSprite = playerSpriteRight;
    newX += 0.1; // Move right & down
    newY += 0.1; // Move up & left
  }

  if (bPressed && !reloading) {
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
  }

  int rPx = round(playerX);
  int rPy = round(playerY);

  // Check if the player reached the exit
  if (dungeonMap[rPy][rPx] == 4) {
    Serial.println("You reached the exit!");
    level += 1;
    statusScreen = true;
  }
}

bool pressed;
int page = 1;
int pageDelay = 0;
void gameOver() {
  bool bPressed = !digitalRead(BUTTON_B_PIN);
  bool aPressed = !digitalRead(BUTTON_A_PIN);

  if (aPressed && !pressed) {
    page++;
    if (page == 3) {
      page = 1;
    }
    pressed = true;
  }

  if (!aPressed) {
    pageDelay = 0;
    pressed = false;
  }

  if (pressed) {
    pageDelay++;
    if (pageDelay >= 50) {
      pageDelay = 0;
      pressed = false;
    }
  }

  char Lvl[7];
  snprintf(Lvl, sizeof(Lvl), "%d", level);
  char KLLS[7];
  snprintf(KLLS, sizeof(KLLS), "%d", kills);
  
  int lvlHighscore;
  lvlHighscore = EEPROM.read(lvlHighscoreAddress);
  if (level > lvlHighscore) {
    EEPROM.write(lvlHighscoreAddress, level);
  }

  int kllHighscore;
  kllHighscore = EEPROM.read(killHighscoreAddress);
  if (kills > kllHighscore) {
    EEPROM.write(killHighscoreAddress, kills);
  }

  char LHighscore[7];
  snprintf(LHighscore, sizeof(LHighscore), "%d", lvlHighscore);
  char KHighscore[7];
  snprintf(KHighscore, sizeof(KHighscore), "%d", kllHighscore);

  u8g2.clearBuffer();
  //Serial.println("You died!");
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(11, 30, "Game over!");

  u8g2.drawFrame(11, 42, 108, 80);

  u8g2.setFont(u8g2_font_profont12_tr);
  if (page == 1) {
    u8g2.drawStr(15, 54, "Slain by:");
    u8g2.drawStr(70, 54, deathCause);

    u8g2.drawStr(15, 66, "On level:");
    u8g2.drawStr(70, 66, Lvl);

    u8g2.drawStr(15, 78, "Lvl highscore:");
    u8g2.drawStr(100, 78, LHighscore);

    u8g2.drawStr(15, 90, "Kills:");
    u8g2.drawStr(52, 90, KLLS);

    u8g2.drawStr(15, 102, "Kll Highscore:");
    u8g2.drawStr(100, 102, KHighscore);

    u8g2.drawStr(15, 114, "[A] next page");
  } else if (page == 2) {
    u8g2.drawStr(15, 54, "next page");

    u8g2.drawStr(15, 114, "[A] next page");
  }

  u8g2.sendBuffer();
  if (bPressed) {
    playerHP = 100;
    level = 1;
    generateDungeon(playerX, playerY, damsel[0]);
    spawnEnemies();
  }
}

void showStatusScreen() {
  bool bPressed = !digitalRead(BUTTON_B_PIN);

  u8g2.clearBuffer();

  if (damsel[0].followingPlayer) {
    u8g2.drawXBMP(0, -10, 128, 128, rescueDamselScreen);
    u8g2.drawStr(0, 125, "You rescued the Damsel!");
  } else if (damsel[0].dead) {
    u8g2.drawXBMP(0, -10, 128, 128, deadDamselScreen);
    u8g2.drawStr(0, 105, "You killed the Damsel...");
    u8g2.drawStr(0, 115, "How could you!");
    u8g2.drawStr(0, 125, "She trusted you!");
  } else if (!damsel[0].dead && !damsel[0].followingPlayer) {
    u8g2.drawStr(0, 125, "You forgot the Damsel!");
  }

  u8g2.sendBuffer();

  if (bPressed && statusScreen) {
    statusScreen = false;
    generateDungeon(playerX, playerY, damsel[0]); // Generate a new dungeon
    spawnEnemies();
  }
}
