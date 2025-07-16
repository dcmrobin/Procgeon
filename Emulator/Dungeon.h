#ifndef DUNGEON_H
#define DUNGEON_H

#include <cstdint>
#include <cmath>
#include "Entities.h"

constexpr int mapWidth = 64;   // Total map width in tiles
constexpr int mapHeight = 64;  // Total map height in tiles
constexpr int tileSize = 8;    // Size of each tile (in pixels)

enum TileTypes {
    StartStairs,
    Floor,
    Wall,
    Bars,
    Exit,
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

// Function declarations
void generateDungeon();
void spawnEnemies();
void setTile(int tileX, int tileY, TileTypes tileType);
void updateScrolling(int viewportWidth, int viewportHeight, float scrollSpeed, float& offsetX, float& offsetY);
void drawMinimap();
void renderDungeon();
void drawTile(int mapX, int mapY, float screenX, float screenY);
int computeTileBrightness(int mapX, int mapY);

// Helper functions
void carveHorizontalCorridor(int x1, int x2, int y);
void carveVerticalCorridor(int y1, int y2, int x);
bool isVisible(int viewerX, int viewerY, int targetX, int targetY);
void drawMinimap();
void renderDungeon();
void drawTile(int mapX, int mapY, float screenX, float screenY);
int computeTileBrightness(int mapX, int mapY);

#endif // DUNGEON_H