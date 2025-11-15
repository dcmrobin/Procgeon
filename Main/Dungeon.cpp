#include <cmath>
#include "WProgram.h"
#include "Dungeon.h"
#include "HelperFunctions.h"
#include "Entities.h"
#include "Player.h"
#include "Item.h"

TileTypes dungeonMap[mapHeight][mapWidth];

int bossfightLevel = 12;
bool generatedMapItem;
bool generatedClockEnemy = false;
bool generatedSuccubusFriend = false;
void generateDungeon(bool isBossfight) {
  generatedMapItem = false;

  // Initialize map with walls
  for (int y = 0; y < mapHeight; y++) {
    for (int x = 0; x < mapWidth; x++) {
      if (!isBossfight) {
        dungeonMap[y][x] = Wall;
      } else {
        dungeonMap[y][x] = (y > 15 && y < mapHeight - 15) && (x > 15 && x < mapWidth - 15) ? Floor : Wall; // Boss fight is an large area
      }
    }
  }

  if (isBossfight) {
    playerX = mapWidth / 2;
    playerY = mapHeight / 2;

    // Only create the damsel's cell if she was following the player and we don't have a friendly succubus
    if (damsel[0].followingPlayer && !damsel[0].dead && !succubusIsFriend && damsel[0].active) {
      // Create a small cell in the top-right corner of the boss room
      int cellX = playerX;
      int cellY = playerY - 10;
      int cellWidth = 5;
      int cellHeight = 5;

      // Create the cell walls
      for (int y = cellY; y < cellY + cellHeight; y++) {
        for (int x = cellX; x < cellX + cellWidth; x++) {
          // Set the cell interior to floor
          dungeonMap[y][x] = Floor;
          // Set walls around the perimeter
          if (x == cellX || x == cellX + cellWidth - 1 || 
              y == cellY || y == cellY + cellHeight - 1) {
            dungeonMap[y][x] = Wall;
          }
        }
      }

      // Add bars in the middle of north and south walls
      int barX = cellX + (cellWidth / 2);
      dungeonMap[cellY][barX] = Bars;              // North wall bars
      dungeonMap[cellY + cellHeight - 1][barX] = Bars;  // South wall bars

      // Place the damsel in the cell
      damsel[0].x = cellX + (cellWidth / 2);
      damsel[0].y = cellY + (cellHeight / 2);
      damsel[0].followingPlayer = false; // She's trapped now
      damsel[0].completelyRescued = true;
    }
    
    return; // Skip regular room generation
  }

  // Room generation parameters
  const int maxRooms = random(15, 25);  // Maximum number of rooms
  const int minRoomSize = 4;            // Minimum room size (tiles)
  const int maxRoomSize = 8;            // Maximum room size (tiles)

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
            // Use rarity-based loot spawning - lower dungeon floors have lower max rarity (3)
            // This makes chest loot more valuable than random floor loot
            dungeonMap[y][x] = getRandomLootTile(1 + dungeon); // The further the player goes, the better the loot
          }
          if (!generatedMapItem && x > roomX + 1 && y > roomY + 1) {
            dungeonMap[y][x] = Map;
            generatedMapItem = true;
          }
        }
      }
    }
  }

  // Spawn chests in random rooms
  int chestCount = random(2, 5); // 2-4 chests per dungeon
  for (int i = 0; i < chestCount; i++) {
    int roomIndex = random(0, roomCount);
    Room &room = rooms[roomIndex];
    int chestX = room.x + random(1, room.width - 1);
    int chestY = room.y + random(1, room.height - 1);
    // Only place if the tile is floor and not occupied by player/damsel/exit
    if (dungeonMap[chestY][chestX] == Floor) {
      dungeonMap[chestY][chestX] = ChestTile;
    }
  }

  // Connect rooms with corridors
  for (int i = 1; i < roomCount; i++) {
    int x1, y1, x2, y2;

    getEdgeTowards(rooms[i - 1], rooms[i], x1, y1);
    getEdgeTowards(rooms[i], rooms[i - 1], x2, y2);

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

  if (dungeon > levelOfDamselDeath + 3 && !succubusIsFriend) {
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

    // Guarantee a closed door at the center of the south wall of the damsel's cell
    int southWallY = damselRoomY + damselRoomHeight - 1;
    int southWallCenterX = damselRoomX + damselRoomWidth / 2;
    dungeonMap[southWallY][southWallCenterX] = DoorClosed;

    // Place the damsel in the cell
    damsel[0].x = centerX - 1;
    damsel[0].y = centerY - 1;
    dungeonMap[centerY][centerX] = ChestTile;
    damsel[0].speed = 0.1;
    damsel[0].followingPlayer = false;
    damsel[0].dead = false;
    damsel[0].active = true;
    damsel[0].completelyRescued = false;
    // Only initialize levelOfLove if this is the first time encountering her
    // If she was recaptured (damselGotTaken), preserve her level of love
    if (!damselGotTaken) {
      damsel[0].levelOfLove = 0;  // First encounter
    }
    // If damselGotTaken is true, her levelOfLove is preserved from before
  } else {
    // Deactivate damsel if succubus is friend or damsel is dead
    damsel[0].x = 3000;
    damsel[0].y = 3000;
    damsel[0].active = false;
    damsel[0].followingPlayer = false;
    damsel[0].completelyRescued = false;
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
      dungeonMap[itemY][itemX] = random(0, 100) > 50 ? ArmorTile : RingTile;
    }
  }

  placeRoomEntranceDoors();
}

// --- Place doors at corridor-to-room transitions (room mouths) ---
void placeRoomEntranceDoors() {
  // Directions: N, S, E, W
  const int dx[4] = {0, 0, 1, -1};
  const int dy[4] = {-1, 1, 0, 0};
  // Perpendicular pairs for corridor check
  const int perp[4][2][2] = {
    {{1, 0}, {-1, 0}}, // N/S: check E/W
    {{1, 0}, {-1, 0}}, // S/N: check E/W
    {{0, -1}, {0, 1}}, // E/W: check N/S
    {{0, -1}, {0, 1}}  // W/E: check N/S
  };
  for (int y = 2; y < mapHeight - 2; y++) {
    for (int x = 2; x < mapWidth - 2; x++) {
      if (dungeonMap[y][x] != Floor) continue;
      // For each direction, check if this is a corridor mouth
      for (int d = 0; d < 4; d++) {
        int bx = x - dx[d];
        int by = y - dy[d];
        int fx = x + dx[d];
        int fy = y + dy[d];
        // Check that behind is corridor (walls on both sides)
        int px1 = x + perp[d][0][0];
        int py1 = y + perp[d][0][1];
        int px2 = x + perp[d][1][0];
        int py2 = y + perp[d][1][1];
        bool corridorBehind = (dungeonMap[by][bx] == Floor || dungeonMap[by][bx] == DoorOpen) &&
                             dungeonMap[py1][px1] == Wall && dungeonMap[py2][px2] == Wall;
        // Ahead is open (room): at least 2 open neighbors not including the corridor
        int openCount = 0;
        for (int pd = 0; pd < 4; pd++) {
          if (pd == (d ^ 1)) continue; // skip the direction we came from
          int nx = fx + dx[pd];
          int ny = fy + dy[pd];
          if (dungeonMap[ny][nx] == Floor || dungeonMap[ny][nx] == DoorOpen || dungeonMap[ny][nx] == DoorClosed) openCount++;
        }
        bool roomAhead = (dungeonMap[fy][fx] == Floor || dungeonMap[fy][fx] == DoorOpen) && openCount >= 2;
        // Only place door if not already a door
        if (corridorBehind && roomAhead && dungeonMap[y][x] == Floor) {
          if (random(0, 6) == 1 || random(0, 6) == 2 || random(0, 6) == 3) {
            dungeonMap[y][x] = DoorClosed;
          } else if (random(0, 6) == 4) {
            dungeonMap[y][x] = DoorOpen;
          } else {
            dungeonMap[y][x] = Floor;
          }
        }
      }
    }
  }
}

void spawnEnemies(bool isBossfight) {
  generatedSuccubusFriend = false;
  generatedClockEnemy = false;
  clockX = -10000;
  clockY = -10000;

  if (!isBossfight) {
    // First, spawn the friendly succubus if we have one
    if (succubusIsFriend && !generatedSuccubusFriend) {
      generatedSuccubusFriend = true;
      currentDamselPortrait = succubusPortrait;
      currentDialogue = "You didn't try to kill me. I'll return the favour.";
      showDialogue = true;
      dialogueTimeLength = 600;
      enemies[0] = { (float)playerX, (float)playerY - 1, 40, false, 0.06, "succubus", 30, 20, false, 0, 0, {}, nullptr, 30, false, true };
      enemies[0].sprite = succubusIdleSprite;
      enemies[0].isFriend = true;
    }
    
    // Then spawn other enemies
    for (int i = (succubusIsFriend ? 1 : 0); i < maxEnemies; i++) {
      while (true) {
        int ex = random(0, mapWidth);
        int ey = random(0, mapHeight);
        if (dungeonMap[ey][ex] == Floor && sqrt(pow(playerX - ex, 2) + pow(playerY - ey, 2)) >= 10) {
          if (random(0, 5) == 1 && dungeon > 1) {
            enemies[i] = { (float)ex, (float)ey, 20, false, 0.05, "blob", 20, 2, false, 0, 0, {}, nullptr, 20, false, false };
            enemies[i].sprite = blobAnimation[random(0, blobAnimationLength)].frame;
          } else if (random(0, 4) == 2 && dungeon > 2) {
            enemies[i] = { (float)ex, (float)ey, 10, false, 0.11, "teleporter", 20, 0, false, 0, 0, {}, nullptr, 20, false, false };
            enemies[i].sprite = teleporterAnimation[random(0, teleporterAnimationLength)].frame;
          } else if (random(0, 6) == 4 && dungeon > 4) {
            enemies[i] = { (float)ex, (float)ey, 15, false, 0.06, "shooter", 20, 0, false, 0, 0, {}, nullptr, 20, false, false };
            enemies[i].sprite = shooterAnimation[random(0, shooterAnimationLength)].frame;
          } else if (random(0, 10) == 5 && dungeon > 6) {
            enemies[i] = { (float)ex, (float)ey, 30, false, 0.02, "succubus", 50, 110, false, 0, 0, {}, nullptr, 50, false, false };
            enemies[i].sprite = succubusIdleSprite;
          } else if (random(0, 12) == 11 && dungeon > 6) {
            if (!generatedClockEnemy) {
              enemies[i] = { (float)ex, (float)ey, 30, false, 0.07, "clock", 20, 0, false, 0, 0, {}, nullptr, 20, false, false };
              enemies[i].sprite = clockAnimation[random(0, clockAnimationLength)].frame;
              generatedClockEnemy = true;
            }
          } else {
            enemies[i] = { (float)ex, (float)ey, 10, false, 0.08, "batguy", 20, 1, false, 0, 0, {}, nullptr, 20, false, false };
            enemies[i].sprite = batguyAnimation[random(0, batguyAnimationLength)].frame;
          }

          /*if (random(0, 30) > 27) {
            enemies[i] = { (float)ex, (float)ey, 50, false, 0.04, "brute", 50, 50, false, 0, 0, false, false };
          }*/
          break;
        }
      }
    }
  } else {
    for (int i = 0; i < maxEnemies; i++) {
      enemies[i] = { (float)0, (float)0, 0, false, 0, "null", 0, 0, false, 0, 0, {}, nullptr, 0, false, false };
      enemies[i].sprite = batguyAnimation[0].frame;
    }

    // Spawn the boss
    enemies[0] = { (float)(mapWidth / 2), (float)(mapHeight / 2), 400, false, 0.04, "boss", 30, 20, false, 0, 0, {}, nullptr, 30, false, false };
    
    // If we have a friendly succubus, spawn her near the player
    if (succubusIsFriend) {
      enemies[1] = { (float)playerX, (float)playerY - 1, 40, false, 0.06, "succubus", 30, 20, false, 0, 0, {}, nullptr, 30, false, true };
      enemies[1].isFriend = true;
      enemies[1].sprite = succubusIdleSprite;
    }
    if (bossState == Idle || bossState == Shooting) {
      enemies[0].sprite = bossIdleAnimation[random(0, bossIdleAnimationLength)].frame;
    } else if (bossState == Floating || bossState == Summoning || bossState == Enraged) {
      enemies[0].sprite = bossFightAnimation[random(0, bossFightAnimationLength)].frame;
    } else if (bossState == Beaten) {
      enemies[0].sprite = bossBeatenAnimation[0].frame;
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
      display.drawBitmap(screenX, screenY, wallSprite, tileSize, tileSize, seeAll ? 15 : brightness);
      break;
    }
    case Bars: {
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      int brightness = computeTileBrightness(mapX, mapY);
      display.drawBitmap(screenX, screenY, barsSprite, tileSize, tileSize, seeAll ? 15 : brightness);
      break;
    }
    case DoorClosed: {
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);
      
      int brightness = computeTileBrightness(mapX, mapY);
      display.drawBitmap(screenX, screenY, doorClosedSprite, tileSize, tileSize, seeAll ? 15 : brightness);
      break;
    }
    case DoorOpen: {
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      int brightness = computeTileBrightness(mapX, mapY);
      display.drawBitmap(screenX, screenY, doorOpenSprite, tileSize, tileSize, seeAll ? 15 : brightness);
      break;
    }
    case StartStairs:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, stairsSprite, tileSize, tileSize, seeAll ? 15 : floorbrightness+10);
      break;
    case Exit:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, stairsSprite, tileSize, tileSize, seeAll ? 15 : floorbrightness+10);
      break;
    case Potion:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, potionSprite, tileSize, tileSize, seeAll ? 15 : floorbrightness+10);
      break;
    case Map:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, mapSprite, tileSize, tileSize, seeAll ? 15 : floorbrightness+10);
      break;
    case MushroomTile:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, mushroomSprite, tileSize, tileSize, seeAll ? 15 : floorbrightness+10);
      break;
    case RiddleStoneTile:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, riddleStoneSprite, tileSize, tileSize, seeAll ? 15 : floorbrightness+10);
      break;
    case ArmorTile:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, armorSprite, tileSize, tileSize, seeAll ? 15 : floorbrightness+10);
      break;
    case ScrollTile:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, scrollSprite, tileSize, tileSize, seeAll ? 15 : floorbrightness+10);
      break;
    case RingTile:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);

      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, ringSprite, tileSize, tileSize, seeAll ? 15 : floorbrightness+10);
      break;
    case Floor:
      display.fillRect(screenX, screenY, tileSize, tileSize, seeAll ? 2 : floorbrightness);
      break;
    case ChestTile:
      display.fillRect(screenX, screenY, tileSize, tileSize, floorbrightness);
      if (isVisible(round(playerX), round(playerY), mapX, mapY))
        display.drawBitmap(screenX, screenY, chestSprite, tileSize, tileSize, seeAll ? 15 : floorbrightness+10);
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