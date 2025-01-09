#include <U8g2lib.h>

// OLED display pins
#define OLED_MOSI 11
#define OLED_CLK 13
#define OLED_DC 7
#define OLED_CS 10
#define OLED_RST 9
#define seedPin A0

const int mapWidth = 32;   // Total map width in tiles
const int mapHeight = 32;  // Total map height in tiles
const int tileSize = 8;    // Size of each tile (in pixels)

// Viewport size (in tiles)
const int viewportWidth = 128 / tileSize;
const int viewportHeight = 128 / tileSize;

// Map scrolling offset
int offsetX = 0;
int offsetY = 0;

// Player position
int playerX = 1;
int playerY = 1;

// Dungeon map (2D array)
int dungeonMap[mapHeight][mapWidth];

// SH1107 128x128 SPI Constructor
U8G2_SH1107_PIMORONI_128X128_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RST);

// Timing variables
unsigned long lastUpdateTime = 0;
const unsigned long frameDelay = 100; // Update every 100ms

void setup() {
  Serial.begin(9600);
  u8g2.begin();

  randomSeed(generateRandomSeed());

  // Generate a random dungeon
  generateDungeon();
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime >= frameDelay) {
    lastUpdateTime = currentTime;

    // Render the visible part of the dungeon
    u8g2.clearBuffer();
    renderDungeon();
    renderPlayer();
    u8g2.sendBuffer();

    // Handle input
    handleInput();
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
  const int maxRooms = random(8, 16);       // Maximum number of rooms
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
    case 1: // Floor
      //u8g2.drawFrame(screenX, screenY, tileSize, tileSize);
      break;
    case 2: // Wall
      u8g2.drawBox(screenX, screenY, tileSize, tileSize);
      break;
    case 4: // Exit
      u8g2.drawCircle(screenX + tileSize / 2, screenY + tileSize / 2, tileSize / 3, U8G2_DRAW_ALL);
      break;
  }
}

// Render the player
void renderPlayer() {
  int screenX = (playerX - offsetX) * tileSize;
  int screenY = (playerY - offsetY) * tileSize;

  // Ensure the player is within the viewport
  if (screenX >= 0 && screenX < 128 && screenY >= 0 && screenY < 128) {
    u8g2.drawDisc(screenX + tileSize / 2, screenY + tileSize / 2, tileSize / 3, U8G2_DRAW_ALL);
  }
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
    if (input == 'a') newX--; // Move left
    if (input == 'd') newX++; // Move right

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
      generateDungeon(); // Generate a new dungeon
    }
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
