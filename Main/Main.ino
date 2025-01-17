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
      updateEnemies();
      updateProjectiles();

      // Render the game
      u8g2.clearBuffer();
      renderDungeon();
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

  u8g2.sendBuffer();
  if (input == 'w' || input == 'a' || input == 's' || input == 'd') {
    playerHP = 100;
    level = 1;
    generateDungeon();
    spawnEnemies();
  }
}