#ifndef DUNGEON_H
#define DUNGEON_H

#include <Arduino.h>
#include "Entities.h"

#define mapWidth 64   // Total map width in tiles
#define mapHeight 64  // Total map height in tiles
#define tileSize 8    // Size of each tile (in pixels)

extern int dungeonMap[mapHeight][mapWidth];

void generateDungeon(float& playerX, float& playerY, Damsel& damsel, int levelOfDamselDeath, int level);
void spawnEnemies(float playerX, float playerY);
void setTile(int tileX, int tileY, int tileType);

#endif