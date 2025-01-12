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
unsigned int highscoreAddress = 0;
byte levelHighscore;
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

// SH1107 128x128 SPI Constructor
U8G2_SH1107_PIMORONI_128X128_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RST);

// Timing variables
unsigned long lastUpdateTime = 0;
const unsigned long frameDelay = 20; // Update every 100ms

void setup() {
  Serial.begin(9600);
  u8g2.begin();
  u8g2.setBitmapMode(1);

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
      updateEnemies();

      // Render the game
      u8g2.clearBuffer();
      renderDungeon();
      renderEnemies();
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

int counter = 0;
void updateAnimations() {
  counter += 1;
  if (counter >= 20) {
    blobSprite = blobSprite == blobSpriteFrame1 ? blobSpriteFrame2 : blobSpriteFrame1;
    counter = 0;
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
  const int maxRooms = random(15, 25);       // Maximum number of rooms
  const int minRoomSize = 4;    // Minimum room size (tiles)
  const int maxRoomSize = 8;    // Maximum room size (tiles)

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

  // Ensure player start
  int playerStartX = startRoomX + startRoomWidth / 2;
  int playerStartY = startRoomY + startRoomHeight / 2;
  dungeonMap[playerStartY][playerStartX] = 1; // Make sure the player's position is a floor
  playerX = playerStartX;
  playerY = playerStartY;

  // Place the exit in the last room
  dungeonMap[rooms[roomCount - 1].y + rooms[roomCount - 1].height / 2]
            [rooms[roomCount - 1].x + rooms[roomCount - 1].width / 2] = 4; // Exit

  dungeonMap[startRoomX + (startRoomWidth/2)][startRoomY + (startRoomHeight/2) + 1] = 0;
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
        enemies[i] = {(float)ex, (float)ey, 20, false, 0.05, "blob", 20}; // Default health is 10
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
      // Move toward the player
      int moveX = 0, moveY = 0;
      if (abs(dx) > abs(dy)) { // Prioritize horizontal movement
        moveX = (dx > 0 ? 1 : -1);
      } else { // Prioritize vertical movement
        moveY = (dy > 0 ? 1 : -1);
      }

      // Calculate potential new position
      float nx = enemies[i].x + moveX * enemies[i].moveAmount;
      float ny = enemies[i].y + moveY * enemies[i].moveAmount;

      int ptx = predictXtile(nx);
      int pty = predictYtile(ny);

      int ctx = round(nx);
      int cty = round(ny);

      if (dungeonMap[cty][ptx] != 1) {
        nx = ptx > ctx ? -enemies[i].moveAmount*2 : ptx < ctx ? enemies[i].moveAmount*2 : nx;
      }
      if (dungeonMap[pty][ctx] != 1) {
        ny = pty > cty ? -enemies[i].moveAmount*2 : pty < cty ? enemies[i].moveAmount*2 : ny;
      }

      // Check if the new position is within bounds and not a wall
      if (nx >= 0 && ny >= 0 && nx < mapWidth && ny < mapHeight && dungeonMap[pty][ptx] == 1) {
        enemies[i].x = nx;
        enemies[i].y = ny;
      }
    } else {
      // Random wandering
      int dir = random(0, 4);
      float nx = enemies[i].x + (dir == 0 ? enemies[i].moveAmount*2 : dir == 1 ? -(enemies[i].moveAmount)*2 : 0);
      float ny = enemies[i].y + (dir == 2 ? enemies[i].moveAmount*2 : dir == 3 ? -(enemies[i].moveAmount)*2 : 0);

      int ptx = predictXtile(nx);
      int pty = predictYtile(ny);

      int ctx = round(nx);
      int cty = round(ny);

      if (dungeonMap[cty][ptx] != 1) {
        nx = ptx > ctx ? -enemies[i].moveAmount*2 : ptx < ctx ? enemies[i].moveAmount*2 : nx;
      }
      if (dungeonMap[pty][ctx] != 1) {
        ny = pty > cty ? -enemies[i].moveAmount*2 : pty < cty ? enemies[i].moveAmount*2 : ny;
      }

      // Check bounds and avoid walls
      if (nx >= 0 && ny >= 0 && nx < mapWidth && ny < mapHeight && dungeonMap[pty][ptx] == 1) {
        enemies[i].x = nx;
        enemies[i].y = ny;
      }


    }

    // Check for collision with the player
    if ((int)enemies[i].x == playerX && (int)enemies[i].y == playerY) {
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
    if (x < (int)x + 0.5f) {  // Slightly lower or exactly the integer
        return (int)x;
    }
    // Slightly higher
    return (int)x + 1;
}

int predictYtile(float y) {
    if (y < (int)y + 0.5f) {  // Slightly lower or exactly the integer
        return (int)y;
    }
    // Slightly higher
    return (int)y + 1;
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

    if (input == 'w') newY--; // Move up
    if (input == 's') newY++; // Move down

    if (input == 'a') {
      playerSprite = playerSpriteLeft;
      newX--;
    }
    if (input == 'd') {
      playerSprite = playerSpriteRight;
      newX++;
    }

    //Serial.println(playerX);
    //Serial.println(playerY);

    // Check collision with walls
    if (dungeonMap[newY][newX] != 2) {
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

void gameOver() {
  char input = Serial.read();
  char Lvl[7];
  snprintf(Lvl, sizeof(Lvl), "%d", level);
  
  int lvlHighscore;
  lvlHighscore = EEPROM.read(highscoreAddress);
  if (level > lvlHighscore) {
    EEPROM.write(highscoreAddress, level);
  }

  char LHighscore[7];
  snprintf(LHighscore, sizeof(LHighscore), "%d", lvlHighscore);

  u8g2.clearBuffer();
  //Serial.println("You died!");
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(11, 30, "Game over!");

  u8g2.drawFrame(11, 42, 108, 80);

  u8g2.setFont(u8g2_font_profont12_tr);
  u8g2.drawStr(15, 54, "Slain by:");
  u8g2.drawStr(70, 54, deathCause);

  u8g2.drawStr(15, 66, "On level:");
  u8g2.drawStr(70, 66, Lvl);

  u8g2.drawStr(15, 78, "Highscore:");
  u8g2.drawStr(76, 78, LHighscore);

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