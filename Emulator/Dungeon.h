#ifndef DUNGEON_H
#define DUNGEON_H

#include <cstdint>
#include <cmath>
#include "Entities.h"

#define mapWidth 64
#define mapHeight 64
#define tileSize 8

enum TileTypes {
  StartStairs,
  Floor,
  Wall,
  Bars,
  DoorClosed,
  DoorOpen,
  Exit,
  KeyTile,
  KeyItem,
  Freedom,
  Potion,
  Map,
  MushroomTile,   // Blanket food tile — picks a random food item on pickup
  RiddleStoneTile,
  ArmorTile,
  ScrollTile,
  RingTile,
  ChestTile,
  WeaponTile,
  GoldTile        // Gold coin — increments goldCount, never goes to inventory
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