#include <cmath>
#include "WProgram.h"
#include "Dungeon.h"
#include "HelperFunctions.h"
#include "Entities.h"
#include "Player.h"

TileTypes dungeonMap[mapHeight][mapWidth];

bool generatedMapItem;
void generateDungeon() {
  generatedMapItem = false;

  // Initialize map with walls
  for (int y = 0; y < mapHeight; y++) {
    for (int x = 0; x < mapWidth; x++) {
      dungeonMap[y][x] = Wall;
    }
  }

  // Room generation parameters
  const int maxRooms = random(15, 25);  // Maximum number of rooms
  const int minRoomSize = 4;            // Minimum room size (tiles)
  const int maxRoomSize = 8;            // Maximum room size (tiles)

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
  rooms[roomCount++] = { startRoomX, startRoomY, startRoomWidth, startRoomHeight };
  for (int y = startRoomY; y < startRoomY + startRoomHeight; y++) {
    for (int x = startRoomX; x < startRoomX + startRoomWidth; x++) {
      dungeonMap[y][x] = Floor;
    }
  }

  // Generate additional rooms
  for (int i = 1; i < maxRooms; i++) {
    int roomWidth = random(minRoomSize, maxRoomSize + 1);
    int roomHeight = random(minRoomSize, maxRoomSize + 1);
    int roomX = random(3, mapWidth - roomWidth - 3);
    int roomY = random(3, mapHeight - roomHeight - 3);

    // Check for overlaps
    bool overlap = false;
    for (int j = 0; j < roomCount; j++) {
      if (roomX < rooms[j].x + rooms[j].width && roomX + roomWidth > rooms[j].x && roomY < rooms[j].y + rooms[j].height && roomY + roomHeight > rooms[j].y) {
        overlap = true;
        break;
      }
    }

    // Add the room if no overlap
    if (!overlap) {
      rooms[roomCount++] = { roomX, roomY, roomWidth, roomHeight };
      for (int y = roomY; y < roomY + roomHeight; y++) {
        for (int x = roomX; x < roomX + roomWidth; x++) {
          dungeonMap[y][x] = Floor;
          if (random(0, 50) > 47) {
            if (random(0, 70) >= 30) {
              dungeonMap[y][x] = MushroomTile;
            } else if (random(0, 70) > 50) {
              dungeonMap[y][x] = Potion;
            } else if (random(0, 70) > 55) {
              dungeonMap[y][x] = ScrollTile;
            } else if (random(0, 70) > 65) {
              dungeonMap[y][x] = RiddleStoneTile;
            }
          }
          if (!generatedMapItem && x > roomX + 1 && y > roomY + 1) {
            dungeonMap[y][x] = Map;
            generatedMapItem = true;
          }
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
      if (dungeonMap[y][x] == Wall) {
        int neighbors = 0;

        for (int nx = -1; nx < 2; nx++) {
          for (int ny = -1; ny < 2; ny++) {
            if (nx == 0 && ny == 0) continue;
            if (dungeonMap[y + ny][x + nx] == Wall) {
              neighbors += 1;
            }
          }
        }

        if (neighbors <= 1) {
          dungeonMap[y][x] = Floor;
        }
      }
    }
  }

  if (dungeon > levelOfDamselDeath + 3) {
    // Generate the damsel's cell
    int damselRoomWidth = 7;
    int damselRoomHeight = 5;
    int damselRoomX, damselRoomY;

    // Find a location far from the start room
    do {
      damselRoomX = random(3, mapWidth - damselRoomWidth - 3);
      damselRoomY = random(3, mapHeight - damselRoomHeight - 3);
    } while (abs(damselRoomX - startRoomX) + abs(damselRoomY - startRoomY) < mapWidth / 2);

    // Create the damsel's cell
    for (int y = damselRoomY; y < damselRoomY + damselRoomHeight; y++) {
      for (int x = damselRoomX; x < damselRoomX + damselRoomWidth; x++) {
        if ((y == damselRoomY || y == damselRoomY + damselRoomHeight - 1) && x >= damselRoomX && x < damselRoomX + damselRoomWidth) {
          dungeonMap[y][x] = Bars;
        } else {
          dungeonMap[y][x] = Floor;
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
    damsel[0].x = centerX - 1;
    damsel[0].y = centerY - 1;
    dungeonMap[centerY][centerX] = Potion;
    damsel[0].speed = 0.1;
    damsel[0].followingPlayer = false;
    damsel[0].dead = false;
    damsel[0].active = true;
  } else {
    damsel[0].x = 3000;
    damsel[0].y = 3000;
    damsel[0].active = false;
  }

  dungeonMap[startRoomX + (startRoomWidth / 2)][startRoomY + (startRoomHeight / 2) + 1] = StartStairs;

  // Ensure player start
  int playerStartX = startRoomX + startRoomWidth / 2;
  int playerStartY = startRoomY + startRoomHeight / 2;
  dungeonMap[playerStartY][playerStartX] = Floor;  // Make sure the player's position is a floor
  playerX = playerStartX;
  playerY = playerStartY;

  // Place the exit in the last room
  dungeonMap[rooms[roomCount - 1].y + rooms[roomCount - 1].height / 2]
            [rooms[roomCount - 1].x + rooms[roomCount - 1].width / 2] = Exit;

  // Spawn some equipment items in random rooms
  int equipmentCount = random(1, 4); // Spawn 1-3 equipment items
  for (int i = 0; i < equipmentCount; i++) {
    int roomIndex = random(1, roomCount - 1); // Don't spawn in start or exit room
    Room &room = rooms[roomIndex];
    
    int itemX = room.x + random(1, room.width - 1);
    int itemY = room.y + random(1, room.height - 1);
    
    // Only place if the tile is floor and not occupied
    if (dungeonMap[itemY][itemX] == Floor) {
      dungeonMap[itemY][itemX] = ArmorTile;
    }
  }
}

void spawnEnemies() {
  for (int i = 0; i < maxEnemies; i++) {
    while (true) {
      int ex = random(0, mapWidth);
      int ey = random(0, mapHeight);
      if (dungeonMap[ey][ex] == Floor && sqrt(pow(playerX - ex, 2) + pow(playerY - ey, 2)) >= 10) {
        if (random(0, 30) <= 24) {
          enemies[i] = { (float)ex, (float)ey, 20, false, 0.05, "blob", 20, 5, false, 0, 0 };
        } else {
          enemies[i] = { (float)ex, (float)ey, 10, false, 0.11, "teleporter", 20, 0, false, 0, 0 };
        }
        break;
      }
    }
  }
}

void setTile(int tileX, int tileY, TileTypes tileType) {
  dungeonMap[tileY][tileX] = tileType;
}

void updateScrolling(int viewportWidth, int viewportHeight, float scrollSpeed, float& offsetX, float& offsetY) {
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

void drawMinimap() {
  display.clearDisplay();
  int mapScale = 2;

  for (int y = 0; y < mapHeight; y++) {
    for (int x = 0; x < mapWidth; x++) {
      TileTypes tile = dungeonMap[y][x];
      int drawX = x * mapScale;
      int drawY = y * mapScale;

      if (tile == Floor) continue;
      if (tile == Wall) display.fillRect(drawX, drawY, mapScale, mapScale, 15);
      if (tile == Bars) display.drawCircle(drawX, drawY, mapScale / 2, 15);
      if (tile == Exit) display.drawRect(drawX, drawY, mapScale, mapScale, 15);
    }
  }

  int playerMinimapX = (playerX)*mapScale;
  int playerMinimapY = (playerY)*mapScale;
  display.drawCircle(playerMinimapX, playerMinimapY, 1, 10);

  if (seeAll) {
    for (int i = 0; i < maxEnemies; i++) {
      int enemyMinimapX = (enemies[i].x)*mapScale;
      int enemyMinimapY = (enemies[i].y)*mapScale;
      display.drawCircle(enemyMinimapX, enemyMinimapY, 1, 7);
    }
    int damselMinimapX = (damsel[0].x)*mapScale;
    int damselMinimapY = (damsel[0].y)*mapScale;
    display.drawCircle(damselMinimapX, damselMinimapY, 2, 15);
  }

  display.display();
}

// Render the visible portion of the dungeon
void renderDungeon() {
  for (int y = 1; y < viewportHeight + 1; y++) {  // +1 to handle partial tiles at edges
    for (int x = 1; x < viewportWidth + 1; x++) {
      float mapX = x + offsetX;
      float mapY = y + offsetY;

      if (mapX >= 0 && mapX < mapWidth && mapY >= 0 && mapY < mapHeight) {
        // Calculate screen position based on fractional offsets
        float screenX = (x - (offsetX - (int)offsetX)) * tileSize;
        float screenY = (y - (offsetY - (int)offsetY)) * tileSize;

        // Draw the tile
        drawTile((int)mapX, (int)mapY, screenX, screenY);
      }
    }
  }
}

void drawTile(int mapX, int mapY, float screenX, float screenY) {
  TileTypes tileType = dungeonMap[mapY][mapX];

  int floorbrightness = computeTileBrightness(mapX, mapY);
  floorbrightness -= 10;
  floorbrightness = floorbrightness < 0 ? 0 : floorbrightness;

  switch (tileType) {
    case Wall: {
      int brightness = computeTileBrightness(mapX, mapY);
      display.drawBitmap(screenX, screenY, wallSprite, tileSize, tileSize, brightness);
      break;
    }
    case Bars: {
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      int brightness = computeTileBrightness(mapX, mapY);
      display.drawBitmap(screenX, screenY, barsSprite, tileSize, tileSize, brightness);
      break;
    }
    case StartStairs:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, stairsSprite, tileSize, tileSize, floorbrightness+10);
      break;
    case Exit:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, stairsSprite, tileSize, tileSize, floorbrightness+10);
      break;
    case Potion:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, potionSprite, tileSize, tileSize, floorbrightness+10);
      break;
    case Map:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, mapSprite, tileSize, tileSize, floorbrightness+10);
      break;
    case MushroomTile:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, mushroomSprite, tileSize, tileSize, floorbrightness+10);
      break;
    case RiddleStoneTile:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, riddleStoneSprite, tileSize, tileSize, floorbrightness+10);
      break;
    case ArmorTile:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, armorSprite, tileSize, tileSize, floorbrightness+10);
      break;
    case ScrollTile:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, scrollSprite, tileSize, tileSize, floorbrightness+10);
      break;
    case Floor:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);
      break;
  }
}

int computeTileBrightness(int mapX, int mapY) {
  int totalNeighbors = 0;
  int litNeighbors = 0;
  
  // Calculate distance from the player
  float dist = sqrt(pow(playerX - mapX, 2) + pow(playerY - mapY, 2));

  // Maximum brightness is 15, minimum is 3 (so it never goes fully dark)
  int maxBrightness = 15;
  int minBrightness = 3;

  // Define a falloff range (adjust this for better results)
  float falloffStart = 5.0f;  // Distance at which brightness starts decreasing
  float falloffEnd = 10.0f;   // Maximum distance where brightness reaches min

  // Compute brightness based on visibility of neighbors
  for (int dx = -1; dx <= 1; dx++) {
    for (int dy = -1; dy <= 1; dy++) {
      if (dx == 0 && dy == 0) continue;  // Skip the tile itself
      int nx = mapX + dx;
      int ny = mapY + dy;
      if (nx >= 0 && nx < mapWidth && ny >= 0 && ny < mapHeight) {
        totalNeighbors++;
        if (isVisible(round(playerX), round(playerY), nx, ny)) {
          litNeighbors++;
        }
      }
    }
  }

  float fraction = totalNeighbors > 0 ? (float)litNeighbors / totalNeighbors : 0.0f;
  int brightness = minBrightness + (int)(fraction * (maxBrightness - minBrightness));

  // Apply distance-based dimming
  if (dist > falloffStart) {
    float factor = 1.0f - ((dist - falloffStart) / (falloffEnd - falloffStart));
    factor = constrain(factor, 0.0f, 1.0f); // Keep factor between 0 and 1
    brightness = minBrightness + (int)(factor * (brightness - minBrightness));
  }

  return brightness;
}