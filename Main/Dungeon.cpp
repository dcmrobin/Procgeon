#include "Dungeon.h"
#include "HelperFunctions.h"
#include "Entities.h"

int dungeonMap[mapHeight][mapWidth];

void generateDungeon(float& playerX, float& playerY, Damsel& damsel) {
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
  damsel.x = centerX-1;
  damsel.y = centerY-1;
  damsel.speed = 0.1;
  damsel.followingPlayer = false;
  damsel.dead = false;

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