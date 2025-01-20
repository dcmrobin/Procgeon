#include <U8g2lib.h>
#include <EEPROM.h>

// OLED display pins
#define OLED_MOSI 11
#define OLED_CLK 13
#define OLED_DC 7
#define OLED_CS 10
#define OLED_RST 9
#define seedPin A0

const int mapWidth = 64;   // Total map width in tiles
const int mapHeight = 64;  // Total map height in tiles
const int tileSize = 8;    // Size of each tile (in pixels)

// Viewport size (in tiles)
const int viewportWidth = 128 / tileSize;
const int viewportHeight = 128 / tileSize - 2;

// Map scrolling offset
int offsetX = 0;
int offsetY = 0;

// Smooth scrolling speed
const float scrollSpeed = 0.5f;

// Player position
int playerX = 1;
int playerY = 1;

// Player stats
int playerHP = 100;
int level = 1;
int kills = 0;
unsigned int lvlHighscoreAddress = 0;
unsigned int killHighscoreAddress = 1;
const char* deathCause = "";

// Dungeon map (2D array)
int dungeonMap[mapHeight][mapWidth];

static const unsigned char PROGMEM stairsSprite[] =
{ 0b11000000,
  0b11000000,
  0b11110000,
  0b11110000,
  0b11111100,//sprite is flipped on its x axis for some reason
  0b11111100,
  0b11111111,
  0b11111111
};

static const unsigned char PROGMEM wallSprite[] =
{ 
  0b00000000, 
  0b11101111, 
  0b00000000, 
  0b10111011, 
  0b10111011, 
  0b00000000, 
  0b11101110, 
  0b11101110
};

static const unsigned char PROGMEM barsSprite[] =
{ 
  0b11111111, 
  0b01110111, 
  0b00100010, 
  0b00100010, 
  0b00100010, 
  0b00100010, 
  0b01110111, 
  0b11111111
};

static const unsigned char PROGMEM playerSpriteRight[] =
{ 
  0b00001100, 
  0b00111000, 
  0b00011100, 
  0b01011000, 
  0b01111100, 
  0b01011100, 
  0b01011100, 
  0b01011110
};

static const unsigned char PROGMEM playerSpriteLeft[] =
{ 
  0b00110000, 
  0b00011100, 
  0b00111000, 
  0b00011010, 
  0b00111110, 
  0b00111010, 
  0b00111010, 
  0b01111010
};

const unsigned char* playerSprite = playerSpriteLeft;

static const unsigned char PROGMEM damselSpriteRight[] =
{ 
  0b00000000, 
  0b00011000, 
  0b00111100, 
  0b00011110, 
  0b00111100, 
  0b01011010, 
  0b00111100, 
  0b01111110
};

static const unsigned char PROGMEM damselSpriteLeft[] =
{ 
  0b00000000, 
  0b00011000, 
  0b00111100, 
  0b01111000, 
  0b00111100, 
  0b01011010, 
  0b00111100, 
  0b01111110
};

static const unsigned char PROGMEM damselHopefullSpriteRight[] =
{ 
  0b00000000, 
  0b00011000, 
  0b00111100, 
  0b00011110, 
  0b01111100, 
  0b00011010, 
  0b00111100, 
  0b01111110
};

static const unsigned char PROGMEM damselHopefullSpriteLeft[] =
{ 
  0b00000000, 
  0b00011000, 
  0b00111100, 
  0b01111000, 
  0b00111110, 
  0b01011000, 
  0b00111100, 
  0b01111110
};

static const unsigned char PROGMEM damselSpriteDead[] =
{ 
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b00001100, 
  0b11011110, 
  0b11111111
};

const unsigned char* damselSprite = damselSpriteLeft;

static const unsigned char PROGMEM blobSpriteFrame1[] =
{ 
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b00111100, 
  0b01011110, 
  0b01111110, 
  0b01111110, 
  0b01111110
};

static const unsigned char PROGMEM blobSpriteFrame2[] =
{ 
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b00111100, 
  0b01011110, 
  0b01111110, 
  0b11111111
};

const unsigned char* blobSprite = blobSpriteFrame1;

struct Enemy {
  float x, y;
  int hp;
  bool chasingPlayer;
  float moveAmount;
  const char* name;
  int attackDelay;
};
const int maxEnemies = 30; // Adjust the number of enemies as needed
Enemy enemies[maxEnemies];

struct Projectile {
  float x, y;
  float dx, dy;
  float speed;
  float damage;
  bool active;
};
const int maxProjectiles = 10;
Projectile projectiles[maxProjectiles];

struct Damsel {
  float x, y;
  float speed;
  bool dead;
  bool followingPlayer;
};
Damsel damsel[1];

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

  for (int i = 0; i < maxProjectiles; i++) {
      projectiles[i].active = false;
  }

  randomSeed(generateRandomSeed());

  // Generate a random dungeon
  generateDungeon();
  spawnEnemies();
}

void loop() {
  unsigned long currentTime = millis();
  if (playerHP > 0) {
    if (currentTime - lastUpdateTime >= frameDelay) {
      lastUpdateTime = currentTime;

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

void generateDungeon() {
  // Initialize map with walls
  for (int y = 0; y < mapHeight; y++) {
    for (int x = 0; x < mapWidth; x++) {
      dungeonMap[y][x] = 2; // Wall
    }
  }

  // Room generation parameters
  const int maxRooms = random(15, 25); // Maximum number of rooms
  const int minRoomSize = 4;           // Minimum room size (tiles)
  const int maxRoomSize = 8;           // Maximum room size (tiles)

  struct Room {
    int x, y, width, height;
  };

  Room rooms[maxRooms];
  int roomCount = 0;

  // Guarantee a starting room at the center of the map
  int startRoomWidth = random(minRoomSize, maxRoomSize + 1);
  int startRoomHeight = random(minRoomSize, maxRoomSize + 1);
  int startRoomX = mapWidth / 2 - startRoomWidth / 2;
  int startRoomY = mapHeight / 2 - startRoomHeight / 2;

  // Create the starting room
  rooms[roomCount++] = {startRoomX, startRoomY, startRoomWidth, startRoomHeight};
  for (int y = startRoomY; y < startRoomY + startRoomHeight; y++) {
    for (int x = startRoomX; x < startRoomX + startRoomWidth; x++) {
      dungeonMap[y][x] = 1; // Floor
    }
  }

  // Generate additional rooms
  for (int i = 1; i < maxRooms; i++) {
    int roomWidth = random(minRoomSize, maxRoomSize + 1);
    int roomHeight = random(minRoomSize, maxRoomSize + 1);
    int roomX = random(1, mapWidth - roomWidth - 1);
    int roomY = random(1, mapHeight - roomHeight - 1);

    // Check for overlaps
    bool overlap = false;
    for (int j = 0; j < roomCount; j++) {
      if (roomX < rooms[j].x + rooms[j].width && roomX + roomWidth > rooms[j].x &&
          roomY < rooms[j].y + rooms[j].height && roomY + roomHeight > rooms[j].y) {
        overlap = true;
        break;
      }
    }

    // Add the room if no overlap
    if (!overlap) {
      rooms[roomCount++] = {roomX, roomY, roomWidth, roomHeight};
      for (int y = roomY; y < roomY + roomHeight; y++) {
        for (int x = roomX; x < roomX + roomWidth; x++) {
          dungeonMap[y][x] = 1; // Floor
        }
      }
    }
  }

  // Connect rooms with corridors
  for (int i = 1; i < roomCount; i++) {
    // Get the center of the current room and the previous room
    int x1 = rooms[i - 1].x + rooms[i - 1].width / 2;
    int y1 = rooms[i - 1].y + rooms[i - 1].height / 2;
    int x2 = rooms[i].x + rooms[i].width / 2;
    int y2 = rooms[i].y + rooms[i].height / 2;

    // Randomly decide corridor order (horizontal-first or vertical-first)
    if (random(0, 2) == 0) {
      carveHorizontalCorridor(x1, x2, y1);
      carveVerticalCorridor(y1, y2, x2);
    } else {
      carveVerticalCorridor(y1, y2, x1);
      carveHorizontalCorridor(x1, x2, y2);
    }
  }

  // This is to get rid of any lone tiles
  for (int y = 0; y < mapHeight; y++) {
    for (int x = 0; x < mapWidth; x++) {
      if (dungeonMap[y][x] == 2) {
        int neighbors = 0;

        for(int nx = -1; nx < 2; nx++) {
          for(int ny = -1; ny < 2; ny++) {
            if (nx == 0 && ny == 0) continue;
            if (dungeonMap[y + ny][x + nx] == 2) {
              neighbors += 1;
            }
          }
        }

        if (neighbors <= 1) {
          dungeonMap[y][x] = 1;
        }
      }
    }
  }

  // Generate the damsel's cell
  int damselRoomWidth = 7;
  int damselRoomHeight = 5;
  int damselRoomX, damselRoomY;

  // Find a location far from the start room
  do {
    damselRoomX = random(1, mapWidth - damselRoomWidth - 1);
    damselRoomY = random(1, mapHeight - damselRoomHeight - 1);
  } while (abs(damselRoomX - startRoomX) + abs(damselRoomY - startRoomY) < mapWidth / 2);

  // Create the damsel's cell
  for (int y = damselRoomY; y < damselRoomY + damselRoomHeight; y++) {
    for (int x = damselRoomX; x < damselRoomX + damselRoomWidth; x++) {
      if ((y == damselRoomY || y == damselRoomY + damselRoomHeight - 1) && x >= damselRoomX && x < damselRoomX + damselRoomWidth) {
        dungeonMap[y][x] = 3; // Walls using barsSprite
      } else {
        dungeonMap[y][x] = 1; // Floor
      }
    }
  }

  // Connect the damsel's cell to the dungeon
  int centerX = damselRoomX + damselRoomWidth / 2;
  int centerY = damselRoomY + damselRoomHeight / 2;
  int startCenterX = startRoomX + startRoomWidth / 2;
  int startCenterY = startRoomY + startRoomHeight / 2;

  carveHorizontalCorridor(startCenterX, centerX, startCenterY);
  carveVerticalCorridor(startCenterY, centerY, centerX);

  // Place the damsel in the cell
  damsel[0].x = centerX;
  damsel[0].y = centerY;
  damsel[0].speed = 0.1;
  damsel[0].followingPlayer = false;
  damsel[0].dead = false;

  dungeonMap[startRoomX + (startRoomWidth/2)][startRoomY + (startRoomHeight/2) + 1] = 0;

  // Ensure player start
  int playerStartX = startRoomX + startRoomWidth / 2;
  int playerStartY = startRoomY + startRoomHeight / 2;
  dungeonMap[playerStartY][playerStartX] = 1; // Make sure the player's position is a floor
  playerX = playerStartX;
  playerY = playerStartY;

  // Place the exit in the last room
  dungeonMap[rooms[roomCount - 1].y + rooms[roomCount - 1].height / 2]
            [rooms[roomCount - 1].x + rooms[roomCount - 1].width / 2] = 4; // Exit
}

// Carve a horizontal corridor
void carveHorizontalCorridor(int x1, int x2, int y) {
  if (x1 > x2) swap(x1, x2);
  for (int x = x1; x <= x2; x++) {
    dungeonMap[y][x] = 1; // Floor
  }
}

// Carve a vertical corridor
void carveVerticalCorridor(int y1, int y2, int x) {
  if (y1 > y2) swap(y1, y2);
  for (int y = y1; y <= y2; y++) {
    dungeonMap[y][x] = 1; // Floor
  }
}

// Utility function to swap values
void swap(int &a, int &b) {
  int temp = a;
  a = b;
  b = temp;
}

// Count surrounding walls for smoothing
int countWalls(int x, int y) {
  int wallCount = 0;
  for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {
      if (dx != 0 || dy != 0) {
        if (dungeonMap[y + dy][x + dx] == 2) {
          wallCount++;
        }
      }
    }
  }
  return wallCount;
}

// Render the visible portion of the dungeon
void renderDungeon() {
  for (int y = 0; y < viewportHeight; y++) {
    for (int x = 0; x < viewportWidth; x++) {
      int mapX = x + offsetX;
      int mapY = y + offsetY;

      if (mapX >= 0 && mapX < mapWidth && mapY >= 0 && mapY < mapHeight) {
        drawTile(mapX, mapY, x * tileSize, y * tileSize);
      }
    }
  }
}

// Draw a tile based on its type
void drawTile(int mapX, int mapY, int screenX, int screenY) {
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
  int screenX = (playerX - offsetX) * tileSize;
  int screenY = (playerY - offsetY) * tileSize;

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
    int dx = playerX - (int)enemies[i].x;
    int dy = playerY - (int)enemies[i].y;
    int distanceSquared = dx * dx + dy * dy;

    // Check if enemy should chase the player
    if (distanceSquared <= 25) { // Chase if within 5 tiles (distance^2 = 25)
      enemies[i].chasingPlayer = true;
    } else {
      enemies[i].chasingPlayer = false;
    }

    if (enemies[i].chasingPlayer) {
      // Move diagonally or straight toward the player
      float moveX = (dx > 0 ? 1 : dx < 0 ? -1 : 0);
      float moveY = (dy > 0 ? 1 : dy < 0 ? -1 : 0);

      // Normalize movement vector to prevent faster diagonal movement
      float magnitude = sqrt(moveX * moveX + moveY * moveY);
      if (magnitude > 0) {
        moveX /= magnitude;
        moveY /= magnitude;
      }

      // Calculate potential new position
      float nx = enemies[i].x + moveX * enemies[i].moveAmount;
      float ny = enemies[i].y + moveY * enemies[i].moveAmount;

      // Check bounds and ensure the move is valid
      if (!checkSpriteCollisionWithTileX(nx, enemies[i].x, ny)) {
        enemies[i].x = nx;
      }
      if (!checkSpriteCollisionWithTileY(ny, enemies[i].y, nx)) {
        enemies[i].y = ny;
      }
    } else {
      // Random wandering
      int dir = random(0, 4);
      float nx = enemies[i].x + (dir == 0 ? enemies[i].moveAmount*2 : dir == 1 ? -(enemies[i].moveAmount)*2 : 0);
      float ny = enemies[i].y + (dir == 2 ? enemies[i].moveAmount*2 : dir == 3 ? -(enemies[i].moveAmount)*2 : 0);

      if (!checkSpriteCollisionWithTileX(nx, enemies[i].x, ny)) {
        enemies[i].x = nx;
      }
      if (!checkSpriteCollisionWithTileY(ny, enemies[i].y, nx)) {
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

int predictXtile(float x) {
  return (int)(x + 0.5f); // Always round to the nearest integer
}

int predictYtile(float y) {
  return (int)(y + 0.5f); // Always round to the nearest integer
}

bool checkSpriteCollisionWithTileX(float newX, float currentX, float newY) {
    int ptx = predictXtile(newX);
    int cty = round(newY);

    bool xValid = (newX >= 0 && newX < mapWidth && dungeonMap[cty][ptx] == 1);

    if (!xValid) {
        newX = currentX;
    }

    return !xValid;
}

bool checkSpriteCollisionWithTileY(float newY, float currentY, float newX) {
    int pty = predictYtile(newY);
    int ctx = round(newX);

    bool yValid = (newY >= 0 && newY < mapHeight && dungeonMap[pty][ctx] == 1);

    if (!yValid) {
        newY = currentY;
    }

    return !yValid;
}

bool checkSpriteCollisionWithSprite(float sprite1X, float sprite1Y, float sprite2X, float sprite2Y) {
  // Use predictXtile/predictYtile for consistent rounding
  int tile1X = predictXtile(sprite1X);
  int tile1Y = predictYtile(sprite1Y);
  int tile2X = predictXtile(sprite2X);
  int tile2Y = predictYtile(sprite2Y);

  return tile1X == tile2X && tile1Y == tile2Y;
}

void renderEnemies() {
  for (int i = 0; i < maxEnemies; i++) {
    if (enemies[i].hp > 0) {
      int screenX = (enemies[i].x - offsetX) * tileSize;
      int screenY = (enemies[i].y - offsetY) * tileSize;
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

  // Calculate distance to player
  int dx = playerX - (int)damsel[0].x;
  int dy = playerY - (int)damsel[0].y;
  int distanceSquared = dx * dx + dy * dy;

  // Check if enemy should chase the player
  if (distanceSquared <= 25) { // Chase if within 5 tiles (distance^2 = 25)
    damsel[0].followingPlayer = true;
    damsel[0].speed = 0.3;
  } else {
    damsel[0].followingPlayer = false;
    damsel[0].speed = 0.1;
  }

  if (!damsel[0].followingPlayer) {
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
    if (damselMoveDelay >= 3) {
      // Move diagonally or straight toward the player
      float moveX = (dx > 0 ? 1 : dx < 0 ? -1 : 0);
      float moveY = (dy > 0 ? 1 : dy < 0 ? -1 : 0);

      damselSprite = moveX == 1 ? damselHopefullSpriteRight : moveX == -1 ? damselHopefullSpriteLeft : damselSprite;

      // Normalize movement vector to prevent faster diagonal movement
      float magnitude = sqrt(moveX * moveX + moveY * moveY);
      if (magnitude > 0) {
        moveX /= magnitude;
        moveY /= magnitude;
      }

      // Calculate potential new position
      float nx = damsel[0].x + moveX * (damsel[0].speed);
      float ny = damsel[0].y + moveY * (damsel[0].speed);

      // Check bounds and ensure the move is valid
      if (!checkSpriteCollisionWithTileX(nx, damsel[0].x, ny)) {
        damsel[0].x = nx;
      }
      if (!checkSpriteCollisionWithTileY(ny, damsel[0].y, nx)) {
        damsel[0].y = ny;
      }
      damselMoveDelay = 0;
    }
  }
}

void renderDamsel() {
  int screenX = (damsel[0].x - offsetX) * tileSize;
  int screenY = (damsel[0].y - offsetY) * tileSize;
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
          int screenX = (projectiles[i].x - offsetX) * tileSize + tileSize/2;
          int screenY = (projectiles[i].y - offsetY) * tileSize + tileSize/2;
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

// Handle player input and update position
void handleInput() {
  // Replace this with actual input handling for buttons or joystick
  if (Serial.available() > 0) {
    char input = Serial.read();
    int newX = playerX;
    int newY = playerY;

    if (input == 'w') {
      playerDY = -1;
      playerDX = 0;
      newY--;//up
    }

    if (input == 's') {
      playerDY = 1;
      playerDX = 0;
      newY++;//down
    }

    if (input == 'a') {
      playerDX = -1;
      playerDY = 0;
      playerSprite = playerSpriteLeft;
      newX--;
    }
    if (input == 'd') {
      playerDX = 1;
      playerDY = 0;
      playerSprite = playerSpriteRight;
      newX++;
    }

    if (input == 'e') {
      shootProjectile(playerDX, playerDY);
    }

    //Serial.println(playerX);
    //Serial.println(playerY);

    // Check collision with walls
    if (dungeonMap[newY][newX] == 1 || dungeonMap[newY][newX] == 4 || dungeonMap[newY][newX] == 0) {
      playerX = newX;
      playerY = newY;

      // Update viewport offset if needed
      if (playerX - offsetX < 2 && offsetX > 0) offsetX--;
      if (playerX - offsetX > viewportWidth - 3 && offsetX < mapWidth - viewportWidth) offsetX++;
      if (playerY - offsetY < 2 && offsetY > 0) offsetY--;
      if (playerY - offsetY > viewportHeight - 3 && offsetY < mapHeight - viewportHeight) offsetY++;
    }

    // Check if the player reached the exit
    if (dungeonMap[playerY][playerX] == 4) {
      Serial.println("You reached the exit!");
      level += 1;
      generateDungeon(); // Generate a new dungeon
      spawnEnemies();
    }
  }
}

int switchDelay = 1500;
int page = 1;
void gameOver() {
  switchDelay--;
  if (switchDelay == 0) {
    page++;
    if (page == 3) {
      page = 1;
    }
    switchDelay = 1500;
  }

  char input = Serial.read();

  char Lvl[7];
  snprintf(Lvl, sizeof(Lvl), "%d", level);
  char KLLS[7];
  snprintf(KLLS, sizeof(KLLS), "%d", kills);
  char Nxtpage[7];
  snprintf(Nxtpage, sizeof(Nxtpage), "%d", switchDelay);
  
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

    u8g2.drawStr(15, 114, "Next page in:");
    u8g2.drawStr(94, 114, Nxtpage);
  } else if (page == 2) {
    u8g2.drawStr(15, 54, "next page");
  }

  u8g2.sendBuffer();
  if (input == 'w' || input == 'a' || input == 's' || input == 'd') {
    playerHP = 100;
    level = 1;
    generateDungeon();
    spawnEnemies();
  }
}

uint32_t generateRandomSeed()
{
  uint8_t  seedBitValue  = 0;
  uint8_t  seedByteValue = 0;
  uint32_t seedWordValue = 0;
 
  for (uint8_t wordShift = 0; wordShift < 4; wordShift++)     // 4 bytes in a 32 bit word
  {
    for (uint8_t byteShift = 0; byteShift < 8; byteShift++)   // 8 bits in a byte
    {
      for (uint8_t bitSum = 0; bitSum <= 8; bitSum++)         // 8 samples of analog pin
      {
        seedBitValue = seedBitValue + (analogRead(seedPin) & 0x01);                // Flip the coin eight times, adding the results together
      }
      delay(1);                                                                    // Delay a single millisecond to allow the pin to fluctuate
      seedByteValue = seedByteValue | ((seedBitValue & 0x01) << byteShift);        // Build a stack of eight flipped coins
      seedBitValue = 0;                                                            // Clear out the previous coin value
    }
    seedWordValue = seedWordValue | (uint32_t)seedByteValue << (8 * wordShift);    // Build a stack of four sets of 8 coins (shifting right creates a larger number so cast to 32bit)
    seedByteValue = 0;                                                             // Clear out the previous stack value
  }
  return (seedWordValue);
 
}
