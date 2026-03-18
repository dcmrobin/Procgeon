#include "Common.h"
#include "GameState.h"
#include "Sprites.h"
#include "HelperFunctions.h"
#include "Player.h"
#include "Item.h"
#include "GameAudio.h"
#include "Entities.h"
#include "Shop.h"

#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// Dungeon.cpp  — map generation, enemy spawning, scrolling, rendering
// ─────────────────────────────────────────────────────────────────────────────

TileTypes dungeonMap[MAP_HEIGHT][MAP_WIDTH];

int  bossfightLevel       = BOSSFIGHT_LEVEL;
bool generatedClockEnemy  = false;

static bool generatedMapItem        = false;
static bool generatedSuccubusFriend = false;

// ─────────────────────────────────────────────────────────────────────────────
// generateDungeon
// ─────────────────────────────────────────────────────────────────────────────
void generateDungeon(bool isBossfight) {
    ambientNoiseLevel = 0;
    generatedMapItem  = false;

    // Initialise map
    for (int y = 0; y < MAP_HEIGHT; y++)
        for (int x = 0; x < MAP_WIDTH; x++)
            dungeonMap[y][x] = isBossfight
                ? ((y > 15 && y < MAP_HEIGHT - 15 && x > 15 && x < MAP_WIDTH - 15)
                   ? Floor : Wall)
                : Wall;

    // ── Boss fight layout ─────────────────────────────────────────────────
    if (isBossfight) {
        playerX = (float)(MAP_WIDTH / 2) - 3.0f;
        playerY = (float)(MAP_HEIGHT / 2);

        bool damselPresent = (damsel[0].followingPlayer || damsel[0].beingCarried) &&
                             !damsel[0].dead && !succubusIsFriend && damsel[0].active;
        if (damselPresent) {
            int cx = (int)playerX, cy = (int)playerY - 10;
            int cw = 5, ch = 5;
            for (int y = cy; y < cy + ch; y++)
                for (int x = cx; x < cx + cw; x++)
                    dungeonMap[y][x] = ((x == cx || x == cx + cw - 1 ||
                                         y == cy || y == cy + ch - 1)
                                        ? Wall : Floor);
            int barX = cx + cw / 2;
            dungeonMap[cy][barX]          = Bars;
            dungeonMap[cy + ch - 1][barX] = Bars;
            damsel[0].x = (float)(cx + cw / 2);
            damsel[0].y = (float)(cy + ch / 2);
            damsel[0].followingPlayer   = false;
            damsel[0].beingCarried      = false;
            damsel[0].completelyRescued = true;
        } else {
            DIDNOTRESCUEDAMSEL = true;
        }
        return;
    }

    // ── Regular dungeon ───────────────────────────────────────────────────
    const int maxRooms = random(MAX_ROOMS_MIN, MAX_ROOMS_MAX + 1);
    int  roomCount = 0;

    // Starting room at map centre
    int srW = random(MIN_ROOM_SIZE, MAX_ROOM_SIZE + 1);
    int srH = random(MIN_ROOM_SIZE, MAX_ROOM_SIZE + 1);
    int srX = MAP_WIDTH  / 2 - srW / 2;
    int srY = MAP_HEIGHT / 2 - srH / 2;
    g_state.rooms[roomCount++] = { srX, srY, srW, srH };
    for (int y = srY; y < srY + srH; y++)
        for (int x = srX; x < srX + srW; x++)
            dungeonMap[y][x] = Floor;

    // Additional rooms
    for (int i = 1; i < maxRooms; i++) {
        int rW = random(MIN_ROOM_SIZE, MAX_ROOM_SIZE + 1);
        int rH = random(MIN_ROOM_SIZE, MAX_ROOM_SIZE + 1);
        int rX = random(3, MAP_WIDTH  - rW - 3);
        int rY = random(3, MAP_HEIGHT - rH - 3);

        bool overlap = false;
        for (int j = 0; j < roomCount && !overlap; j++) {
            overlap = rX < g_state.rooms[j].x + g_state.rooms[j].width  &&
                      rX + rW > g_state.rooms[j].x              &&
                      rY < g_state.rooms[j].y + g_state.rooms[j].height &&
                      rY + rH > g_state.rooms[j].y;
        }
        if (overlap) continue;

        g_state.rooms[roomCount++] = { rX, rY, rW, rH };
        for (int y = rY; y < rY + rH; y++) {
            for (int x = rX; x < rX + rW; x++) {
                dungeonMap[y][x] = Floor;
                if (random(0, 70) > 67)
                    dungeonMap[y][x] = MushroomTile;
                if (random(0, 100) > 97)
                    dungeonMap[y][x] = getRandomLootTile(1 + dungeon);
                if (!generatedMapItem && x > rX + 1 && y > rY + 1) {
                    dungeonMap[y][x] = Map;
                    generatedMapItem = true;
                }
            }
        }
    }

    // Chests
    for (int i = 0; i < random(CHEST_SPAWN_COUNT_MIN, CHEST_SPAWN_COUNT_MAX + 1); i++) {
        Room& r = g_state.rooms[random(0, roomCount)];
        int cx  = r.x + random(1, r.width  - 1);
        int cy  = r.y + random(1, r.height - 1);
        if (dungeonMap[cy][cx] == Floor) dungeonMap[cy][cx] = ChestTile;
    }

    // ── Damsel cell ───────────────────────────────────────────────────────
    if (dungeon > levelOfDamselDeath + 3 && !succubusIsFriend && !endlessMode) {
        int dW = 7, dH = 5, dX, dY;
        do {
            dX = random(3, MAP_WIDTH  - dW - 3);
            dY = random(3, MAP_HEIGHT - dH - 3);
        } while (abs(dX - srX) + abs(dY - srY) < MAP_WIDTH / 2);

        int barX = dX + dW / 2;
        for (int y = dY; y < dY + dH; y++) {
            for (int x = dX; x < dX + dW; x++) {
                if (y == dY || y == dY + dH - 1) {
                    dungeonMap[y][x] = (y == dY && x == barX) ? Floor : Bars;
                } else {
                    dungeonMap[y][x] = Floor;
                }
            }
        }

        int centerX = dX + dW / 2, centerY = dY + dH / 2;
        int sCX     = srX + srW / 2, sCY = srY + srH / 2;
        carveHorizontalCorridor(sCX, centerX, sCY);
        carveVerticalCorridor(sCY, centerY, centerX);

        dungeonMap[dY + dH - 1][dX + dW / 2] = DoorClosed;

        damsel[0].x = (float)(centerX - 1);
        damsel[0].y = (float)(centerY - 1);
        dungeonMap[centerY][centerX] = ChestTile;
        damsel[0].speed              = 0.1f;
        damsel[0].followingPlayer    = false;
        damsel[0].beingCarried       = false;
        damsel[0].dead               = false;
        damsel[0].active             = true;
        damsel[0].completelyRescued  = false;
    } else {
        damsel[0].x = -3000.0f; damsel[0].y = -3000.0f;
        damsel[0].active          = false;
        damsel[0].beingCarried    = false;
        damsel[0].followingPlayer = false;
        damsel[0].completelyRescued = false;
    }

    // ── Connect rooms ─────────────────────────────────────────────────────
    for (int i = 1; i < roomCount; i++) {
        int x1, y1, x2, y2;
        getEdgeTowards(g_state.rooms[i - 1], g_state.rooms[i], x1, y1);
        getEdgeTowards(g_state.rooms[i],     g_state.rooms[i - 1], x2, y2);
        if (random(0, 2) == 0) {
            carveHorizontalCorridor(x1, x2, y1);
            carveVerticalCorridor(y1, y2, x2);
        } else {
            carveVerticalCorridor(y1, y2, x1);
            carveHorizontalCorridor(x1, x2, y2);
        }
    }

    // Place shop in an already existing room
    if (random(0, 10) < 2) {
        setupShopItems();
        g_state.shopOnThisFloor = true;
        Room& r = g_state.rooms[7];
        int sx  = r.x + random(1, r.width  - 1);
        int sy  = r.y + random(1, r.height - 1);
        for (int y = r.y-1; y <= r.y + r.height; y++) {
            for (int x = r.x-1; x <= r.x + r.width; x++) {
                if ((y == r.y - 1 || x == r.x - 1 || y == r.y + r.height || x == r.x + r.width) && dungeonMap[y][x] != DoorClosed && dungeonMap[y][x] != DoorOpen && dungeonMap[y][x] != Floor) {
                    dungeonMap[y][x] = ShopWall;
                }

                // Generate line of kiosk tiles in the middle of the room
                if (y == (r.y + r.height / 2)-1 && x > r.x && x < r.x + r.width - 1 && dungeonMap[y][x] == Floor) {
                    dungeonMap[y][x] = Kiosk;
                }
            }
        }
    }

    // ── Remove lone wall tiles ────────────────────────────────────────────
    for (int y = 1; y < MAP_HEIGHT - 1; y++)
        for (int x = 1; x < MAP_WIDTH - 1; x++)
            if (dungeonMap[y][x] == Wall && countWalls(x, y) <= 1)
                dungeonMap[y][x] = Floor;

    // ── Player start ──────────────────────────────────────────────────────
    dungeonMap[srX + srW / 2][srY + srH / 2 + 1] = StartStairs;
    int psX = srX + srW / 2, psY = srY + srH / 2;
    dungeonMap[psY][psX] = Floor;
    playerX = (float)psX;
    playerY = (float)psY;

    // ── Exit ─────────────────────────────────────────────────────────────
    int exY = g_state.rooms[roomCount - 1].y + g_state.rooms[roomCount - 1].height / 2;
    int exX = g_state.rooms[roomCount - 1].x + g_state.rooms[roomCount - 1].width  / 2;
    dungeonMap[exY][exX] = Exit;

    // Optional locked exit
    int keyChance = 30 + dungeon;
    if (random(0, 100) < keyChance) {
        dungeonMap[exY][exX] = KeyTile;
        bool placed = false;
        for (int attempt = 0; attempt < 100 && !placed; attempt++) {
            Room& r  = g_state.rooms[random(1, roomCount - 1)];
            int   kx = r.x + random(1, r.width  - 1);
            int   ky = r.y + random(1, r.height - 1);
            if (dungeonMap[ky][kx] == Floor) {
                dungeonMap[ky][kx] = KeyItem;
                placed = true;
            }
        }
        if (!placed) {
            // Fallback: place key in start room
            for (int y = srY; y < srY + srH && !placed; y++)
                for (int x = srX; x < srX + srW && !placed; x++)
                    if (dungeonMap[y][x] == Floor) {
                        dungeonMap[y][x] = KeyItem;
                        placed = true;
                    }
        }
    }

    // Equipment spawns
    /*for (int i = 0; i < random(1, 4); i++) {
        Room& r = g_state.rooms[random(1, roomCount - 1)];
        int ix  = r.x + random(1, r.width  - 1);
        int iy  = r.y + random(1, r.height - 1);
        if (dungeonMap[iy][ix] == Floor)
            dungeonMap[iy][ix] = (random(0, 100) > 50) ? ArmorTile : RingTile; //no, we do not want this. loot spawning is handled farther up the code.
    }*/

    placeRoomEntranceDoors();
}

// ─────────────────────────────────────────────────────────────────────────────
// placeRoomEntranceDoors
// ─────────────────────────────────────────────────────────────────────────────
void placeRoomEntranceDoors() {
    const int ddx[4] = { 0, 0, 1,-1};
    const int ddy[4] = {-1, 1, 0, 0};
    const int perp[4][2][2] = {
        {{1,0},{-1,0}}, {{1,0},{-1,0}},
        {{0,-1},{0,1}}, {{0,-1},{0,1}}
    };

    for (int y = 2; y < MAP_HEIGHT - 2; y++) {
        for (int x = 2; x < MAP_WIDTH - 2; x++) {
            if (dungeonMap[y][x] != Floor) continue;
            for (int d = 0; d < 4; d++) {
                int bx = x - ddx[d], by = y - ddy[d];
                int fx = x + ddx[d], fy = y + ddy[d];
                int p1x = x + perp[d][0][0], p1y = y + perp[d][0][1];
                int p2x = x + perp[d][1][0], p2y = y + perp[d][1][1];

                bool corridorBehind = (dungeonMap[by][bx] == Floor ||
                                       dungeonMap[by][bx] == DoorOpen) &&
                                       dungeonMap[p1y][p1x] == Wall &&
                                       dungeonMap[p2y][p2x] == Wall;

                int openCount = 0;
                for (int pd = 0; pd < 4; pd++) {
                    if (pd == (d ^ 1)) continue;
                    int nx = fx + ddx[pd], ny = fy + ddy[pd];
                    if (dungeonMap[ny][nx] == Floor  ||
                        dungeonMap[ny][nx] == DoorOpen ||
                        dungeonMap[ny][nx] == DoorClosed) openCount++;
                }
                bool roomAhead = (dungeonMap[fy][fx] == Floor ||
                                  dungeonMap[fy][fx] == DoorOpen) && openCount >= 2;

                if (corridorBehind && roomAhead && dungeonMap[y][x] == Floor) {
                    int r = random(0, 6);
                    if      (r <= 2) dungeonMap[y][x] = DoorClosed;
                    else if (r == 3) dungeonMap[y][x] = DoorOpen;
                    else             dungeonMap[y][x] = Floor;
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// spawnEnemies
// ─────────────────────────────────────────────────────────────────────────────
void spawnEnemies(bool isBossfight) {
    generatedSuccubusFriend = false;
    generatedClockEnemy     = false;
    clockX = -10000.0f;
    clockY = -10000.0f;

    // Helper: endless-mode stat scaling
    auto scale = [](int base, int extra) -> int {
        return base + (endlessMode ? extra : 0);
    };
    auto dmgScale = [](int base, int dungeon_) -> int {
        return base + (endlessMode ? (dungeon_ - (dungeon_ - 15 < 0 ? 0 : 15)) : 0);
    };

    if (!isBossfight) {
        // Friendly succubus if active
        if (succubusIsFriend && !generatedSuccubusFriend) {
            generatedSuccubusFriend = true;
            currentDamselPortrait = succubusPortrait;
            snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                     "%s", "You didn't try to kill me. I'll return the favour.");
            playRawSFX(24);
            showDialogue = true;
            dialogueTimeLength = 600;
            enemies[0] = { (float)playerX, playerY - 1.0f, 40, false, 0.06f,
                           "succubus", 30, 20, false, 0, 0, {}, nullptr, 30, false, true };
            enemies[0].sprite  = succubusIdleSprite;
            enemies[0].isFriend= true;
        }

        int start = succubusIsFriend ? 1 : 0;
        for (int i = start; i < MAX_ENEMIES; i++) {
            while (true) {
                int ex = random(0, MAP_WIDTH);
                int ey = random(0, MAP_HEIGHT);
                if (dungeonMap[ey][ex] != Floor) continue;
                float dx = (float)(ex) - playerX, dy = (float)(ey) - playerY;
                if (sqrtf(dx * dx + dy * dy) < ENEMY_SPAWN_MIN_DIST) continue;

                if        (random(0, 5) == 1 && dungeon > 1) {
                    enemies[i] = { (float)ex, (float)ey, scale(20, dungeon - 11),
                                   false, 0.05f, "blob", 20, dmgScale(2, dungeon),
                                   false, 0, 0, {}, nullptr, 20, false, false };
                    enemies[i].sprite = blobAnimation[random(0, blobAnimationLength)].frame;
                } else if (random(0, 4) == 2 && dungeon > 3) {
                    enemies[i] = { (float)ex, (float)ey, scale(10, dungeon - 11),
                                   false, 0.11f, "teleporter", 20, dmgScale(0, dungeon),
                                   false, 0, 0, {}, nullptr, 20, false, false };
                    enemies[i].sprite = teleporterAnimation[random(0, teleporterAnimationLength)].frame;
                } else if (random(0, 6) == 4 && dungeon > 4) {
                    enemies[i] = { (float)ex, (float)ey, scale(15, dungeon - 11),
                                   false, 0.06f, "shooter", 20, dmgScale(0, dungeon),
                                   false, 0, 0, {}, nullptr, 20, false, false };
                    enemies[i].sprite = shooterAnimation[random(0, shooterAnimationLength)].frame;
                } else if (random(0, 10) == 5 && dungeon > 6) {
                    enemies[i] = { (float)ex, (float)ey, scale(30, dungeon - 11),
                                   false, 0.02f, "succubus", 50, dmgScale(110, dungeon),
                                   false, 0, 0, {}, nullptr, 50, false, false };
                    enemies[i].sprite = succubusIdleSprite;
                } else if (random(0, 100) > 92 && dungeon > 2) {
                    enemies[i] = { (float)ex, (float)ey, scale(25, dungeon - 11),
                                   false, 0.07f, "jukebox", 20, dmgScale(0, dungeon),
                                   false, 0, 0, {}, nullptr, 20, false, false };
                    enemies[i].sprite = jukeboxAnimation[random(0, jukeboxAnimationLength)].frame;
                } else if (random(0, 12) == 11 && dungeon > 6 && !generatedClockEnemy) {
                    enemies[i] = { (float)ex, (float)ey, scale(30, dungeon - 11),
                                   false, 0.07f, "clock", 20, dmgScale(0, dungeon),
                                   false, 0, 0, {}, nullptr, 20, false, false };
                    enemies[i].sprite      = clockAnimation[random(0, clockAnimationLength)].frame;
                    generatedClockEnemy    = true;
                } else {
                    enemies[i] = { (float)ex, (float)ey, scale(10, dungeon - 11),
                                   false, 0.08f, "batguy", 20, dmgScale(1, dungeon),
                                   false, 0, 0, {}, nullptr, 20, false, false };
                    enemies[i].sprite = batguyAnimation[random(0, batguyAnimationLength)].frame;
                }
                break;
            }
        }

        if (g_state.shopOnThisFloor) { // If a shop is on this floor, then spawn the shopkeeper in the shop
            Room& r = g_state.rooms[7];
            int   sx = r.x + r.width / 2;
            int   sy = (r.y + r.height / 2)-2; // Spawn shopkeeper just above the line of kiosks
            if (dungeonMap[sy][sx] == Floor) {
                enemies[7] = { ((float)sx), ((float)sy), 1000, false, 0.0f,
                               "shopkeeper", 1000 /*He can't die*/, 1, false, 0, 0, {}, nullptr, 9999, false, true };
                enemies[7].sprite = shopkeeperSprite;
            }
        }
    } else {
        // Boss fight — clear all enemies first
        for (int i = 0; i < MAX_ENEMIES; i++) {
            enemies[i] = { 0.0f, 0.0f, 0, false, 0.0f, "null",
                           0, 0, false, 0, 0, {}, nullptr, 0, false, false };
            enemies[i].sprite = batguyAnimation[0].frame;
        }

        enemies[0] = { (float)(MAP_WIDTH / 2), (float)(MAP_HEIGHT / 2),
                       BOSS_START_HP, false, 0.04f, "boss",
                       30, BOSS_CONTACT_DAMAGE_NORMAL,
                       false, 0, 0, {}, nullptr, 30, false, false };

        if (succubusIsFriend) {
            enemies[1] = { (float)playerX, playerY - 1.0f, 40, false, 0.06f,
                           "succubus", 30, 20, false, 0, 0, {}, nullptr, 30, false, true };
            enemies[1].isFriend = true;
            enemies[1].sprite   = succubusIdleSprite;
        }

        // Boss sprite based on current state
        switch (bossState) {
            case Idle:
            case Shooting:
                enemies[0].sprite = bossIdleAnimation[0].frame; break;
            case Floating:
            case Summoning:
            case Enraged:
                enemies[0].sprite = bossFightAnimation[0].frame; break;
            case Beaten:
                enemies[0].sprite = bossBeatenAnimation[0].frame; break;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Misc
// ─────────────────────────────────────────────────────────────────────────────
void setTile(int tileX, int tileY, TileTypes tileType) {
    dungeonMap[tileY][tileX] = tileType;
}

void updateScrolling(int vpW, int vpH, float scrollSpd, float& offX, float& offY) {
    float targetX = playerX - (vpW / 2.0f) + 0.5f;
    float targetY = playerY - (vpH / 2.0f) + 0.5f;
    targetX = constrain(targetX, 0.0f, (float)(MAP_WIDTH  - vpW));
    targetY = constrain(targetY, 0.0f, (float)(MAP_HEIGHT - vpH));
    offX += (targetX - offX) * scrollSpd;
    offY += (targetY - offY) * scrollSpd;
}

// ─────────────────────────────────────────────────────────────────────────────
// drawMinimap
// ─────────────────────────────────────────────────────────────────────────────
void drawMinimap() {
    display.clearDisplay();
    const int scale = 2;

    if (blinded) {
        display.setCursor(10, 10);
        display.print("You can't see the map while blinded!");
        display.display();
        return;
    }

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            int dx = x * scale, dy = y * scale;
            switch (dungeonMap[y][x]) {
                case Wall:    display.fillRect(dx, dy, scale, scale, 15); break;
                case Bars:    display.drawCircle(dx, dy, scale / 2, 15);  break;
                case Exit:    display.drawRect(dx, dy, scale, scale, 15); break;
                case KeyTile: display.drawRoundRect(dx, dy, scale, scale, 1, 10); break;
                case KeyItem: display.fillRect(dx + 1, dy + 1, scale - 2, scale - 2, 10); break;
                case ShopWall: display.drawCircle(dx, dy, scale / 2, 15);  break;
                default: break;
            }
        }
    }

    display.drawCircle((int)(playerX * scale), (int)(playerY * scale), 1, 10);

    if (seeAll) {
        for (int i = 0; i < MAX_ENEMIES; i++)
            if (enemies[i].hp > 0)
                display.drawCircle((int)(enemies[i].x * scale),
                                   (int)(enemies[i].y * scale), 1, 7);
        display.drawCircle((int)(damsel[0].x * scale),
                           (int)(damsel[0].y * scale), 2, 15);
    }
    display.display();
}

// ─────────────────────────────────────────────────────────────────────────────
// renderDungeon / drawTile / computeTileBrightness
// ─────────────────────────────────────────────────────────────────────────────
void renderDungeon() {
    int ptx = round(playerX), pty = round(playerY);

    for (int y = 0; y < viewportHeight + 2; y++) {
        for (int x = 0; x < viewportWidth + 2; x++) {
            float mapX = x - 1 + offsetX;
            float mapY = y - 1 + offsetY;
            if (mapX < 0 || mapX >= MAP_WIDTH || mapY < 0 || mapY >= MAP_HEIGHT) continue;

            if (invisibleRingsNumber > 0 && !seeAll) {
                float dx = mapX - playerX, dy = mapY - playerY;
                if (sqrtf(dx * dx + dy * dy) > 3.0f) continue;
            }

            float sx = (x - 1 - (offsetX - (int)offsetX)) * tileSize;
            float sy = (y - 1 - (offsetY - (int)offsetY)) * tileSize;
            drawTile((int)mapX, (int)mapY, sx, sy);
        }
    }
}

void drawTile(int mapX, int mapY, float screenX, float screenY) {
    TileTypes t  = dungeonMap[mapY][mapX];
    int bright   = computeTileBrightness(mapX, mapY);
    int floorBrt = bright - 10;
    if (floorBrt < 0) floorBrt = 0;

    bool vis = seeAll || isVisible(round(playerX), round(playerY), mapX, mapY);

    switch (t) {
        case Wall:
            display.drawBitmap((int)screenX, (int)screenY, wallSprite, tileSize, tileSize,
                               seeAll ? 15 : bright);
            break;
        case ShopWall:
            display.drawBitmap((int)screenX, (int)screenY, shopWallSprite, tileSize, tileSize,
                               seeAll ? 15 : bright);
            break;
        case Kiosk:
            display.drawBitmap((int)screenX, (int)screenY, kioskSprite, tileSize, tileSize,
                               seeAll ? 15 : bright);
            break;
        case Bars:
            display.fillRect((int)screenX, (int)screenY, tileSize, tileSize, floorBrt);
            display.drawBitmap((int)screenX, (int)screenY, barsSprite, tileSize, tileSize,
                               seeAll ? 15 : bright);
            break;
        case DoorClosed:
            display.fillRect((int)screenX, (int)screenY, tileSize, tileSize, floorBrt);
            display.drawBitmap((int)screenX, (int)screenY, doorClosedSprite, tileSize, tileSize,
                               seeAll ? 15 : bright);
            break;
        case DoorOpen:
            display.fillRect((int)screenX, (int)screenY, tileSize, tileSize, floorBrt);
            display.drawBitmap((int)screenX, (int)screenY, doorOpenSprite, tileSize, tileSize,
                               seeAll ? 15 : bright);
            break;
        case Floor:
            display.fillRect((int)screenX, (int)screenY, tileSize, tileSize,
                             seeAll ? 2 : floorBrt);
            break;
        // All item/special tiles share the same pattern: fill floor, draw sprite if visible
        default: {
            display.fillRect((int)screenX, (int)screenY, tileSize, tileSize, floorBrt);
            if (!vis) break;
            const unsigned char* spr = nullptr;
            switch (t) {
                case StartStairs:
                case Exit:
                case Freedom:      spr = stairsSprite;       break;
                case KeyTile:      spr = lockedSprite;        break;
                case KeyItem:      spr = keySprite;           break;
                case Potion:       spr = potionSprite;        break;
                case Map:          spr = mapSprite;           break;
                case MushroomTile: spr = mushroomSprite;      break;
                case RiddleStoneTile: spr = riddleStoneSprite; break;
                case ArmorTile:    spr = armorSprite;         break;
                case ScrollTile:   spr = scrollSprite;        break;
                case RingTile:     spr = ringSprite;          break;
                case ChestTile:    spr = chestSprite;         break;
                case WeaponTile:   spr = weaponSprite;        break;
                case GoldTile:     spr = goldSprite;          break;
                default: break;
            }
            if (spr)
                display.drawBitmap((int)screenX, (int)screenY, spr, tileSize, tileSize,
                                   seeAll ? 15 : floorBrt + 10);
            break;
        }
    }
}

int computeTileBrightness(int mapX, int mapY) {
    float dist      = sqrtf((playerX - mapX) * (playerX - mapX) +
                             (playerY - mapY) * (playerY - mapY));
    float fallStart = (invisibleRingsNumber > 0 && !seeAll) ? 0.0f : 5.0f;
    float fallEnd   = (invisibleRingsNumber > 0 && !seeAll) ? 3.0f : 10.0f;

    int total = 0, lit = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (!dx && !dy) continue;
            int nx = mapX + dx, ny = mapY + dy;
            if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT) {
                total++;
                if (isVisible(round(playerX), round(playerY), nx, ny)) lit++;
            }
        }
    }

    float frac   = total > 0 ? (float)lit / total : 0.0f;
    int   bright = 3 + (int)(frac * 12.0f);

    if (dist > fallStart) {
        float factor = 1.0f - ((dist - fallStart) / (fallEnd - fallStart));
        factor  = constrain(factor, 0.0f, 1.0f);
        bright  = 3 + (int)(factor * (bright - 3));
    }
    return bright;
}