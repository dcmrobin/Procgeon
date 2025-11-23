#ifndef DUNGEON_H
#define DUNGEON_H

#include <Arduino.h>
#include "Entities.h"

#define mapWidth 64   // Total map width in tiles
#define mapHeight 64  // Total map height in tiles
#define tileSize 8    // Size of each tile (in pixels)

enum TileTypes {
  StartStairs,
  Floor,
  Wall,
  Bars,
  DoorClosed,
  DoorOpen,
  Exit,
  Freedom,
  Potion,
  Map,
  MushroomTile,
  RiddleStoneTile,
  ArmorTile,
  ScrollTile,
  RingTile,
  ChestTile
};

extern TileTypes dungeonMap[mapHeight][mapWidth];

extern bool generatedClockEnemy;

extern int bossfightLevel;

struct Room {
  int x, y, width, height;
};

void generateDungeon(bool isBossfight);
void placeRoomEntranceDoors();
void spawnEnemies(bool isBossfight);
void setTile(int tileX, int tileY, TileTypes tileType);
void updateScrolling(int viewportWidth, int viewportHeight, float scrollSpeed, float& offsetX, float& offsetY);
void drawMinimap();
void renderDungeon();
void drawTile(int mapX, int mapY, float screenX, float screenY);
int computeTileBrightness(int mapX, int mapY);

#endif