#ifndef DUNGEON_H
#define DUNGEON_H

#include "Common.h"
#include "GameState.h"   // for Room struct

// ─────────────────────────────────────────────────────────────────────────────
// The dungeon tile map (defined in Dungeon.cpp)
// ─────────────────────────────────────────────────────────────────────────────
extern TileTypes dungeonMap[MAP_HEIGHT][MAP_WIDTH];

extern bool generatedClockEnemy;
extern int  bossfightLevel;

// ─────────────────────────────────────────────────────────────────────────────
// Dungeon generation & rendering
// ─────────────────────────────────────────────────────────────────────────────
void generateDungeon(bool isBossfight);
void placeRoomEntranceDoors();
void spawnEnemies(bool isBossfight);
void setTile(int tileX, int tileY, TileTypes tileType);
void updateScrolling(int vpWidth, int vpHeight, float scrollSpd, float& offX, float& offY);
void drawMinimap();
void renderDungeon();
void drawTile(int mapX, int mapY, float screenX, float screenY);
int  computeTileBrightness(int mapX, int mapY);

#endif // DUNGEON_H