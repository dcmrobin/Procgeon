#include "Common.h"
#include "GameState.h"
#include "Sprites.h"
#include "HelperFunctions.h"
#include "Dungeon.h"
#include "Player.h"
#include "GameAudio.h"
#include "Item.h"

#include <cmath>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
// Entities.cpp
//
// Damsel AI, enemy AI (A* pathfinding, chase/wander, attack),
// projectile update, particle system, rendering for all entities.
// ─────────────────────────────────────────────────────────────────────────────

// ── Global entity arrays ─────────────────────────────────────────────────────
Damsel     damsel[1];
Enemy      enemies[MAX_ENEMIES];
Projectile projectiles[MAX_PROJECTILES];
Particle   particles[MAX_PARTICLES];

float clockX = -10000.0f;
float clockY = -10000.0f;

// ── Dialogue tables ──────────────────────────────────────────────────────────
Dialogue damselAnnoyingDialogue[] = {
    {"Why are you even down here, anyway?",               400},
    {"Don't die.",                                        400},
    {"I hate this place!",                                300, "annoying"},
    {"Why do wizards wear that stupid hat?",              400, "annoying"},
    {"Hurry up, will you?",                               300},
    {"You expect a thanks, don't you? Not gonna happen.", 400, "annoying"},
    {"Does this place ever end?",                         350},
    {"Just saying, I don't like you.",                    400, "annoying"},
    {"When was the last time you showered?",              400, "annoying"},
    {"I could probably get out on my own.",               400},
};

Dialogue damselPassiveDialogue[] = {
    {"Maybe you're not as dumb as I thought.",            400},
    {"Maybe you know where you're going after all...",    400},
    {"Sorry if I said anything hurtful.",                 400, "alone"},
    {"Please don't die.",                                 300, "alone"},
    {"I probably couldn't get out on my own.",            400},
    {"Thanks for rescuing me, anyway.",                   400, "alone"},
    {"Well, at least I'm not alone anymore.",             500},
};

Dialogue damselGoodDialogue[] = {
    {"Actually, I do kind of like you.",                  400, "alone"},
    {"I'm sorry if I was annoying you.",                  400},
    {"Do you mind... carrying me? (Hold X next to me)",   400, "alone"},
    {"You can't kill me, I think.",                       350},
    {"I'm glad I'm with you.",                            300},
    {"I don't want to die.",                              300, "annoying"},
    {"Do you think we're almost at the end?",             400},
};

Dialogue damselCarryDialogue[] = {
    {"You're actually kind of strong...",                 450},
    {"Can I stay in your arms for a bit?",                450},
    {"You can put me down if you want.",                  500},
    {"I kind of like it here...",                         300},
    {"Hm...",                                             300},
    {"I don't want to be in that cell again!",            500},
    {"Can I stay with you after we escape?",              500},
};

Dialogue ridiculeDialogue[] = {
    {"There you have it. You're an idiot.",               400},
    {"Lights are on, but nobody's home.",                 400},
    {"Use your brain! Oh wait- you don't have one.",      400},
    {"Nice move, man.",                                   400},
    {"You're so done for.",                               400},
    {"You're pretty bad at this.",                        400},
    {"Incredibly bad.",                                   400},
    {"Only stupid people drink that potion.",             400},
};

Dialogue glamourDialogue[] = {
    {"Nobody could resist your charm!",                   400},
    {"What bulging muscles!",                             400},
    {"What a glorious hat you wear!",                     400},
    {"You're so handsome!",                               400},
    {"You're the best at this!",                          400},
    {"You're such a hero!",                               400},
    {"Such bravery!",                                     400},
    {"Don't give up, champion!",                          400},
};

// ─────────────────────────────────────────────────────────────────────────────
// A* pathfinding
// ─────────────────────────────────────────────────────────────────────────────
struct Coord { int x, y; };

static bool computePath(int startX, int startY, int goalX, int goalY,
                        PathNode* path, int& pathLength,
                        int maxPathNodes = ASTAR_MAX_NODES) {
    int  g[MAP_HEIGHT][MAP_WIDTH];
    int  f[MAP_HEIGHT][MAP_WIDTH];
    bool closed[MAP_HEIGHT][MAP_WIDTH];
    bool inOpen[MAP_HEIGHT][MAP_WIDTH];
    int  parentX[MAP_HEIGHT][MAP_WIDTH];
    int  parentY[MAP_HEIGHT][MAP_WIDTH];

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            g[y][x]       = ASTAR_INF;
            f[y][x]       = ASTAR_INF;
            closed[y][x]  = false;
            inOpen[y][x]  = false;
            parentX[y][x] = -1;
            parentY[y][x] = -1;
        }
    }

    Coord openList[MAP_WIDTH * MAP_HEIGHT];
    int   openCount = 0;

    g[startY][startX] = 0;
    f[startY][startX] = (abs(goalX - startX) + abs(goalY - startY)) * ASTAR_COST_STRAIGHT;
    openList[openCount++] = { startX, startY };
    inOpen[startY][startX] = true;

    const int dx[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
    const int dy[8] = { -1,-1,-1,  0, 0,  1, 1, 1 };

    bool found = false;
    int  curX = startX, curY = startY;

    while (openCount > 0) {
        // Pick lowest-f node
        int lowestIdx = 0;
        for (int i = 1; i < openCount; i++)
            if (f[openList[i].y][openList[i].x] < f[openList[lowestIdx].y][openList[lowestIdx].x])
                lowestIdx = i;

        Coord cur = openList[lowestIdx];
        curX = cur.x; curY = cur.y;
        openList[lowestIdx] = openList[--openCount];
        inOpen[curY][curX]  = false;
        closed[curY][curX]  = true;

        if (curX == goalX && curY == goalY) { found = true; break; }

        for (int i = 0; i < 8; i++) {
            int nx = curX + dx[i];
            int ny = curY + dy[i];
            if (nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT) continue;
            if (!isWalkable(nx, ny)) continue;
            if (closed[ny][nx]) continue;

            int moveCost = (dx[i] != 0 && dy[i] != 0)
                           ? ASTAR_COST_DIAGONAL : ASTAR_COST_STRAIGHT;
            int tg = g[curY][curX] + moveCost;

            if (!inOpen[ny][nx] || tg < g[ny][nx]) {
                parentX[ny][nx] = curX;
                parentY[ny][nx] = curY;
                g[ny][nx] = tg;
                f[ny][nx] = tg + (abs(goalX - nx) + abs(goalY - ny)) * ASTAR_COST_STRAIGHT;
                if (!inOpen[ny][nx]) {
                    openList[openCount++] = { nx, ny };
                    inOpen[ny][nx] = true;
                }
            }
        }
    }

    if (!found) return false;

    // Reconstruct path (goal → start, then reverse)
    PathNode temp[MAP_WIDTH * MAP_HEIGHT];
    int tempLen = 0;
    int cx = goalX, cy = goalY;
    while (!(cx == startX && cy == startY) && tempLen < MAP_WIDTH * MAP_HEIGHT) {
        temp[tempLen++] = { cx, cy };
        int px = parentX[cy][cx];
        int py = parentY[cy][cx];
        cx = px; cy = py;
    }
    temp[tempLen++] = { startX, startY };

    int toCopy = (tempLen < maxPathNodes) ? tempLen : maxPathNodes;
    for (int i = 0; i < toCopy; i++)
        path[i] = temp[tempLen - 1 - i];
    pathLength = toCopy;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// updateDamsel
// ─────────────────────────────────────────────────────────────────────────────
static int damselMoveDelay = 0;

void updateDamsel() {
    if (damsel[0].dead) {
        damsel[0].followingPlayer = false;
        return;
    }
    damselMoveDelay++;

    // Where the damsel wants to be (just behind the player)
    float destX = (playerDX ==  1) ? playerX - 1
                : (playerDX == -1) ? playerX + 1 : playerX;
    float destY = (playerDY ==  1) ? playerY - 1
                : (playerDY == -1) ? playerY + 1 : playerY;

    if (nearSuccubus) {
        damsel[0].followingPlayer = false;
        destX = damsel[0].x;
        destY = damsel[0].y;
    }

    int idx = round(destX) - round(damsel[0].x);
    int idy = round(destY) - round(damsel[0].y);
    int distSq = idx * idx + idy * idy;

    if (damsel[0].beingCarried) {
        damsel[0].x = playerX;
        damsel[0].y = playerY;
        return;
    }

    // Follow / not follow decision
    if (invisibleRingsNumber == 0) {
        damsel[0].followingPlayer = (distSq <= 25 + damsel[0].levelOfLove * 2);
        damsel[0].speed           = damsel[0].followingPlayer ? 0.3f : 0.1f;
    } else {
        damsel[0].followingPlayer = false;
        damsel[0].speed           = 0.1f;
    }

    // "Hey! Wait up!" — fire once when damsel stops following
    bool succubusChasing = false;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].hp > 0 && strcmp(enemies[i].name, "succubus") == 0 &&
            enemies[i].chasingPlayer && !enemies[i].isFriend) {
            succubusChasing = true; break;
        }
    }

    if (!damsel[0].completelyRescued && !nearSuccubus && !succubusChasing &&
        damselWasFollowing && !damsel[0].followingPlayer &&
        !damselSaidWaitUp  && damselWaitUpTimer <= 0 &&
        !damselGotTaken    && damsel[0].active) {
        currentDamselPortrait = damselPortraitScared;
        dialogueTimeLength    = 300;
        playRawSFX3D(17, damsel[0].x, damsel[0].y);
        snprintf(currentDialogue, sizeof(g_state.currentDialogue), "%s", "Hey! Wait up!");
        showDialogue        = true;
        damselSaidWaitUp    = true;
        damselWaitUpTimer   = DAMSEL_WAIT_UP_COOLDOWN;
    }

    damselWasFollowing = damsel[0].followingPlayer;
    if (damselWaitUpTimer > 0 && --damselWaitUpTimer == 0)
        damselSaidWaitUp = false;

    if (!damsel[0].followingPlayer) {
        // Random wander every 30 ticks
        if (damselMoveDelay >= 30) {
            int dir = random(0, 4);
            float nx = damsel[0].x + ((dir == 0) ?  damsel[0].speed
                                    : (dir == 1) ? -damsel[0].speed : 0.0f);
            float ny = damsel[0].y + ((dir == 2) ?  damsel[0].speed
                                    : (dir == 3) ? -damsel[0].speed : 0.0f);
            damselSprite = (dir == 0) ? damselSpriteRight
                         : (dir == 1) ? damselSpriteLeft : damselSprite;
            if (!checkSpriteCollisionWithTileX(nx, damsel[0].x, ny)) damsel[0].x = nx;
            if (!checkSpriteCollisionWithTileY(ny, damsel[0].y, nx)) damsel[0].y = ny;
            damselMoveDelay = 0;
        }
    } else {
        // First follow greeting
        if (damsel[0].levelOfLove == 0) {
            currentDamselPortrait = damselPortraitNormal;
            dialogueTimeLength    = 400;
            playRawSFX(16);
            snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                     "%s", "Hey! I shall follow you, please get me out of here.");
            showDialogue = true;
            damsel[0].levelOfLove = 1;
        }

        if (damselGotTaken && damselSayThanksForRescue && !succubusIsFriend) {
            playRawSFX(17);
            currentDamselPortrait = damselPortraitAlone;
            dialogueTimeLength    = 400;
            snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                     "%s", "He- wasn't gentle...");
            showDialogue             = true;
            damselSayThanksForRescue = false;
        }

        // Smooth follow movement every 3 ticks
        if (damselMoveDelay >= 3) {
            float moveX = (float)((idx > 0) - (idx < 0));
            float moveY = (float)((idy > 0) - (idy < 0));
            float mag   = sqrtf(moveX * moveX + moveY * moveY);
            if (mag > 0.0f) { moveX /= mag; moveY /= mag; }

            damselSprite = (moveX >  0.5f) ? damselHopefullSpriteRight
                         : (moveX < -0.5f) ? damselHopefullSpriteLeft : damselSprite;

            float nx = damsel[0].x + moveX * damsel[0].speed;
            float ny = damsel[0].y + moveY * damsel[0].speed;
            bool xOk = !checkSpriteCollisionWithTileX(nx, damsel[0].x, damsel[0].y);
            bool yOk = !checkSpriteCollisionWithTileY(ny, damsel[0].y, damsel[0].x);

            if      (xOk && yOk) { damsel[0].x = nx; damsel[0].y = ny; }
            else if (xOk)        { damsel[0].x = nx; }
            else if (yOk)        { damsel[0].y = ny; }
            else {
                // Wall slide
                float sx = damsel[0].x + moveX * damsel[0].speed;
                if (!checkSpriteCollisionWithTileX(sx, damsel[0].x, damsel[0].y))
                    damsel[0].x = sx;
                float sy = damsel[0].y;
                if (!checkSpriteCollisionWithTileY(sy, damsel[0].y, damsel[0].x))
                    damsel[0].y = sy;
            }
            damselMoveDelay = 0;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// updateEnemies
// ─────────────────────────────────────────────────────────────────────────────
static int giveUpTimer = 0;

void updateEnemies() {
    // ── Find clock enemy position ─────────────────────────────────────────
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].hp > 0 && strcmp(enemies[i].name, "clock") == 0) {
            clockX = enemies[i].x;
            clockY = enemies[i].y;
            break;
        }
    }

    // ── Mark enemies near clock ───────────────────────────────────────────
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (strcmp(enemies[i].name, "boss")  == 0) continue;
        if (strcmp(enemies[i].name, "clock") == 0) continue;
        if (enemies[i].hp <= 0) continue;
        float cdx = enemies[i].x - clockX;
        float cdy = enemies[i].y - clockY;
        enemies[i].nearClock = (sqrtf(cdx * cdx + cdy * cdy) <= 5.0f);
    }

    // ── Jukebox volume ────────────────────────────────────────────────────
    {
        float closestDist = 1e9f;
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (enemies[j].hp > 0 && strcmp(enemies[j].name, "jukebox") == 0) {
                float dx = enemies[j].x - playerX;
                float dy = enemies[j].y - playerY;
                float d  = sqrtf(dx * dx + dy * dy);
                if (d < closestDist) closestDist = d;
            }
        }
        float vol = 0.0f;
        if (closestDist < JUKEBOX_MAX_RADIUS)
            vol = 1.0f - (closestDist / JUKEBOX_MAX_RADIUS);
        setJukeboxVolume(vol);
        if (ambientNoiseLevel < (int)(vol * JUKEBOX_NOISE_MAX))
            ambientNoiseLevel = (int)(vol * JUKEBOX_NOISE_MAX);
    }

    // ── Shop volume ─────────────────────────────────────────────────────
    {
        float closestDist = 1e9f;
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (enemies[j].hp > 0 && strcmp(enemies[j].name, "shopkeeper") == 0) {
                float dx = enemies[j].x - playerX;
                float dy = enemies[j].y - playerY;
                float d  = sqrtf(dx * dx + dy * dy);
                if (d < closestDist) closestDist = d;
            }
        }
        float vol = 0.0f;
        if (closestDist < JUKEBOX_MAX_RADIUS)
            vol = 1.0f - (closestDist / JUKEBOX_MAX_RADIUS);
        setShopVolume(vol);
    }

    // ── Per-enemy update ──────────────────────────────────────────────────
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].hp <= 0) continue;

        unstuckEnemy(enemies[i]);

        if (strcmp(enemies[i].name, "boss") == 0) {
            // Let friendly enemies attack the boss
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (j == i || enemies[j].hp <= 0 || !enemies[j].isFriend) continue;
                float dist = sqrtf((enemies[i].x - enemies[j].x) * (enemies[i].x - enemies[j].x) +
                                   (enemies[i].y - enemies[j].y) * (enemies[i].y - enemies[j].y));
                if (dist < 0.5f && enemies[j].attackDelayCounter >= enemies[j].attackDelay) {
                    enemies[i].hp -= enemies[j].damage;
                    if (enemies[i].hp <= 0) kills++;
                    playRawSFX3D(23, enemies[i].x, enemies[i].y);
                    enemies[j].attackDelayCounter = 0;
                }
            }
            continue;
        }

        // Skip if neither player acted nor near clock (performance gate)
        if (!playerActed && !enemies[i].nearClock &&
            strcmp(enemies[i].name, "clock") != 0) continue;

        float diffX = enemies[i].x - playerX;
        float diffY = enemies[i].y - playerY;
        float dist  = sqrtf(diffX * diffX + diffY * diffY);

        // Clock: set playerNearClockEnemy
        if (strcmp(enemies[i].name, "clock") == 0) {
            playerNearClockEnemy = (dist <= 5.0f);
        }

        // Projectile dodging for enemies in line-of-fire
        if (dist > 0.0f && dist * dist < 64.0f) {
            float dot      = diffX * playerDX + diffY * playerDY;
            float cosAngle = dot / dist;

            if (cosAngle > 0.95f) {
                int ex = round(enemies[i].x);
                int ey = round(enemies[i].y);
                int freeDir = 0;
                for (int ddx = -1; ddx <= 1; ddx++)
                    for (int ddy = -1; ddy <= 1; ddy++)
                        if ((ddx || ddy) && dungeonMap[ey + ddy][ex + ddx] == Floor)
                            freeDir++;

                if (freeDir >= 3) {
                    float awayX = enemies[i].x - playerX;
                    float awayY = enemies[i].y - playerY;
                    float plx = -(float)playerDY, ply = (float)playerDX;  // left perp
                    float prx =  (float)playerDY, pry = -(float)playerDX; // right perp
                    float dL = plx * awayX + ply * awayY;
                    float dR = prx * awayX + pry * awayY;
                    float avoidX = (dL >= dR) ? plx : prx;
                    float avoidY = (dL >= dR) ? ply : pry;

                    float toX = -diffX / dist, toY = -diffY / dist;
                    float cx  = avoidX * 0.7f + toX * 0.3f;
                    float cy  = avoidY * 0.7f + toY * 0.3f;
                    float cm  = sqrtf(cx * cx + cy * cy);
                    if (cm > 0.0f) { cx /= cm; cy /= cm; }

                    float nx = enemies[i].x + cx * enemies[i].moveAmount;
                    float ny = enemies[i].y + cy * enemies[i].moveAmount;
                    bool xOk = !checkSpriteCollisionWithTileX(nx, enemies[i].x, enemies[i].y);
                    bool yOk = !checkSpriteCollisionWithTileY(ny, enemies[i].y, enemies[i].x);
                    if      (xOk && yOk) { enemies[i].x = nx; enemies[i].y = ny; }
                    else if (xOk)        { enemies[i].x = nx; }
                    else if (yOk)        { enemies[i].y = ny; }
                    continue;
                }
            }
        }

        // Chase / wander decision
        int egx = round(enemies[i].x), egy = round(enemies[i].y);
        int pgx = round(playerX),       pgy = round(playerY);
        int gdx = pgx - egx, gdy = pgy - egy;
        int gridDistSq = gdx * gdx + gdy * gdy;

        if (enemies[i].isFriend && enemies[i].name != "shopkeeper") {
            enemies[i].chasingPlayer = (invisibleRingsNumber == 0 && gridDistSq > 4);
            if (!enemies[i].chasingPlayer && invisibleRingsNumber == 0) {
                // Attack nearby hostile enemies
                for (int j = 0; j < MAX_ENEMIES; j++) {
                    if (j == i || enemies[j].hp <= 0 || enemies[j].isFriend) continue;
                    int tdx = round(enemies[j].x) - egx;
                    int tdy = round(enemies[j].y) - egy;
                    if (tdx * tdx + tdy * tdy <= 25) {
                        enemies[i].chasingPlayer = true;
                        pgx = round(enemies[j].x);
                        pgy = round(enemies[j].y);
                        break;
                    }
                }
            }
        } else {
            if (invisibleRingsNumber == 0) {
                if (gridDistSq <= CHASE_RANGE_BASE + (int)(ambientNoiseLevel/2)) {
                    enemies[i].chasingPlayer = true;
                } else if (enemies[i].chasingPlayer) {
                    if (++giveUpTimer >= ENEMY_GIVE_UP_TICKS) {
                        if (aggravateRingsNumber > 0) {
                            giveUpTimer = ENEMY_GIVE_UP_TICKS;
                        } else {
                            enemies[i].chasingPlayer = false;
                            giveUpTimer = 0;
                        }
                    }
                }
            } else {
                enemies[i].chasingPlayer = false;
                giveUpTimer = 0;
            }
        }

        // Movement
        if (enemies[i].chasingPlayer) {
            PathNode dynPath[ASTAR_MAX_NODES];
            int      dynLen = 0;
            if (computePath(egx, egy, pgx, pgy, dynPath, dynLen) && dynLen > 1) {
                float tx = (float)dynPath[1].x, ty = (float)dynPath[1].y;
                float mx = tx - enemies[i].x,   my = ty - enemies[i].y;
                float mm = sqrtf(mx * mx + my * my);
                if (mm > 0.0f) { mx = mx / mm * enemies[i].moveAmount;
                                  my = my / mm * enemies[i].moveAmount; }
                float nx = enemies[i].x + mx, ny = enemies[i].y + my;
                bool xOk = !checkSpriteCollisionWithTileX(nx, enemies[i].x, enemies[i].y);
                bool yOk = !checkSpriteCollisionWithTileY(ny, enemies[i].y, enemies[i].x);
                if      (xOk && yOk) { enemies[i].x = nx; enemies[i].y = ny; }
                else if (xOk)        { enemies[i].x = nx; }
                else if (yOk)        { enemies[i].y = ny; }
            }
        } else {
            if (strcmp(enemies[i].name, "succubus") == 0) continue;

            if (!enemies[i].hasWanderPath) {
                int dx = egx + random(-5, 6);
                int dy = egy + random(-5, 6);
                dx = (dx < 0) ? 0 : (dx >= MAP_WIDTH  ? MAP_WIDTH  - 1 : dx);
                dy = (dy < 0) ? 0 : (dy >= MAP_HEIGHT ? MAP_HEIGHT - 1 : dy);
                if (isWalkable(dx, dy) &&
                    computePath(egx, egy, dx, dy,
                                enemies[i].wanderPath, enemies[i].pathLength)) {
                    enemies[i].currentPathIndex = 0;
                    enemies[i].hasWanderPath    = true;
                }
            }

            if (enemies[i].currentPathIndex < enemies[i].pathLength) {
                int nx2 = enemies[i].wanderPath[enemies[i].currentPathIndex].x;
                int ny2 = enemies[i].wanderPath[enemies[i].currentPathIndex].y;
                if (egx == nx2 && egy == ny2) {
                    if (++enemies[i].currentPathIndex >= enemies[i].pathLength)
                        enemies[i].hasWanderPath = false;
                } else {
                    float mx = (float)nx2 - enemies[i].x;
                    float my = (float)ny2 - enemies[i].y;
                    float mm = sqrtf(mx * mx + my * my);
                    if (mm > 0.0f) { mx = mx / mm * enemies[i].moveAmount;
                                     my = my / mm * enemies[i].moveAmount; }
                    float nx = enemies[i].x + mx, ny = enemies[i].y + my;
                    bool xOk = !checkSpriteCollisionWithTileX(nx, enemies[i].x, enemies[i].y);
                    bool yOk = !checkSpriteCollisionWithTileY(ny, enemies[i].y, enemies[i].x);
                    if      (xOk && yOk) { enemies[i].x = nx; enemies[i].y = ny; }
                    else if (xOk)        { enemies[i].x = nx; }
                    else if (yOk)        { enemies[i].y = ny; }
                }
            } else {
                enemies[i].hasWanderPath = false;
            }
        }

        // Shooter fires at player
        if (strcmp(enemies[i].name, "shooter") == 0 && enemies[i].chasingPlayer) {
            static int shooterCooldown[MAX_ENEMIES] = {};
            if (shooterCooldown[i] <= 0) {
                float sdx = playerX - enemies[i].x;
                float sdy = playerY - enemies[i].y;
                float sd  = sqrtf(sdx * sdx + sdy * sdy);
                if (sd > 0.0f) {
                    shootProjectile(enemies[i].x, enemies[i].y,
                                    sdx / sd, sdy / sd, false, i);
                    playRawSFX(1);
                    shooterCooldown[i] = 120;
                }
            } else {
                shooterCooldown[i]--;
            }
        }

        // Contact / combat
        bool isAttacking = false, hasAttacked = false;

        if (checkSpriteCollisionWithSprite(playerX, playerY, enemies[i].x, enemies[i].y)) {
            if (strcmp(enemies[i].name, "teleporter") == 0) {
                if (equippedArmor.item != MagicRobe) {
                    playRawSFX(14);
                    int nx, ny;
                    do { nx = random(0, MAP_WIDTH); ny = random(0, MAP_HEIGHT); }
                    while (dungeonMap[ny][nx] != Floor);
                    playerX = (float)nx; playerY = (float)ny;
                }
            } else if (!enemies[i].isFriend && invisibleRingsNumber == 0) {
                isAttacking = true;
                if (enemies[i].attackDelayCounter >= enemies[i].attackDelay) {
                    int dmg = enemies[i].damage - (round(equippedArmorValue) + armorRingsNumber);
                    if (equippedArmor.item == SpikyArmor) enemies[i].hp -= dmg * 2;
                    if (armorRingsNumber == 0) reduceArmorDurability(i);
                    if (dmg < 0) dmg = 0;
                    if (dmg > 0) {
                        playerHP -= dmg;
                        triggerScreenShake(2, 1);
                        playRawSFX(0);
                        checkIfDeadFrom(enemies[i].name);
                    }
                    hasAttacked = true;
                }
            }
        }

        // Enemy repulsion + friendly combat
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (j == i || enemies[j].hp <= 0) continue;
            float repDist = sqrtf((enemies[i].x - enemies[j].x) * (enemies[i].x - enemies[j].x) +
                                  (enemies[i].y - enemies[j].y) * (enemies[i].y - enemies[j].y));
            if (repDist < REPEL_DISTANCE) {
                float rx = enemies[i].x - enemies[j].x;
                float ry = enemies[i].y - enemies[j].y;
                float rm = sqrtf(rx * rx + ry * ry) + 0.01f;
                enemies[i].x += (rx / rm) * REPEL_STRENGTH;
                enemies[i].y += (ry / rm) * REPEL_STRENGTH;

                if (enemies[i].isFriend && !enemies[j].isFriend && enemies[i].name != "shopkeeper") {
                    isAttacking = true;
                    if (enemies[i].attackDelayCounter >= enemies[i].attackDelay && !hasAttacked) {
                        enemies[j].hp -= enemies[i].damage;
                        if (enemies[j].hp <= 0) kills++;
                        playRawSFX3D(23, enemies[i].x, enemies[i].y);
                        hasAttacked = true;
                    }
                }
            }
        }

        if      (hasAttacked)  enemies[i].attackDelayCounter = 0;
        else if (isAttacking && enemies[i].attackDelayCounter < enemies[i].attackDelay)
                               enemies[i].attackDelayCounter++;
        else                   enemies[i].attackDelayCounter = enemies[i].attackDelay;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// updateProjectiles
// ─────────────────────────────────────────────────────────────────────────────
void updateProjectiles() {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!projectiles[i].active) continue;

        projectiles[i].x += projectiles[i].dx * projectiles[i].speed;
        projectiles[i].y += projectiles[i].dy * projectiles[i].speed;

        int ptx = predictXtile(projectiles[i].x);
        int pty = predictYtile(projectiles[i].y);

        // Wall / out-of-bounds
        bool oob = (projectiles[i].x < 0 || projectiles[i].y < 0 ||
                    projectiles[i].x >= MAP_WIDTH || projectiles[i].y >= MAP_HEIGHT ||
                    projectiles[i].speed <= 0.0f ||
                    (projectiles[i].dx == 0.0f && projectiles[i].dy == 0.0f));
        TileTypes pt = dungeonMap[pty][ptx];
        if (oob || pt == Wall || pt == Bars || pt == DoorClosed || pt == ShopWall || pt == Kiosk) {
            spawnParticles(projectiles[i].x, projectiles[i].y, 3, 0.15f, false);
            projectiles[i].active = false;
            playRawSFX3D(22, projectiles[i].x, projectiles[i].y);
            continue;
        }

        // Enemy hits
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!projectiles[i].shotByPlayer && j == projectiles[i].shooterId) continue;

            bool hit = false;
            if (strcmp(enemies[j].name, "boss") == 0) {
                hit = checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y,
                          enemies[j].x, enemies[j].y) ||
                      checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y,
                          enemies[j].x + 1, enemies[j].y) ||
                      checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y,
                          enemies[j].x, enemies[j].y + 1) ||
                      checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y,
                          enemies[j].x + 1, enemies[j].y + 1);
            } else if (!enemies[j].isFriend) {
                hit = checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y,
                          enemies[j].x, enemies[j].y);
            }

            if (hit && enemies[j].hp > 0 && enemies[i].name != "shopkeeper") {
                enemies[j].hp -= (int)projectiles[i].damage;
                spawnParticles(enemies[j].x, enemies[j].y, 1, 0.15f, false);
                playRawSFX3D(23, enemies[j].x, enemies[j].y);
                if (enemies[j].hp <= 0) {
                    spawnParticles(enemies[j].x, enemies[j].y, 5, 0.25f, true);
                    kills++;
                    if (strcmp(enemies[j].name, "clock") == 0) {
                        enemies[j].x = -3000.0f;
                        enemies[j].y = -3000.0f;
                        playerNearClockEnemy = false;
                    }
                }
                projectiles[i].active = false;
                break;
            }

            // Damsel hit
            if (!damsel[0].dead && !damsel[0].beingCarried &&
                dungeon != bossfightLevel &&
                checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y,
                                               damsel[0].x, damsel[0].y)) {
                if (damsel[0].levelOfLove < 6 || !projectiles[i].shotByPlayer) {
                    if (!projectiles[i].shotByPlayer)
                        snprintf(damselDeathMsg, sizeof(g_state.damselDeathMsg),
                                 "%s", "Something killed ");
                    playRawSFX3D(23, damsel[0].x, damsel[0].y);
                    playRawSFX3D(17, damsel[0].x, damsel[0].y);
                    levelOfDamselDeath     = dungeon;
                    damsel[0].dead         = true;
                    currentDamselPortrait  = damselPortraitDying;
                    dialogueTimeLength     = 200;
                    snprintf(currentDialogue, sizeof(g_state.currentDialogue), "%s", "Ugh-!");
                    showDialogue           = true;
                    damsel[0].active       = false;
                    projectiles[i].active  = false;
                    break;
                }
            }
        }

        // Player hit
        if (!projectiles[i].shotByPlayer &&
            checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y,
                                           playerX, playerY)) {
            int dmg = (int)projectiles[i].damage -
                      (int)(round(equippedArmorValue) + armorRingsNumber);
            if (armorRingsNumber == 0) reduceArmorDurability(0);
            if (dmg < 0) dmg = 0;
            playerHP -= dmg;
            triggerScreenShake(2, 1);
            playRawSFX(0);
            checkIfDeadFrom("projectile");
            projectiles[i].active = false;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Misc entity helpers
// ─────────────────────────────────────────────────────────────────────────────
void moveDamselToPos(float posX, float posY) {
    damsel[0].x = posX;
    damsel[0].y = posY;
}

void reduceArmorDurability(int i) {
    float reduction = (i >= 0 && i < MAX_ENEMIES)
                      ? (float)enemies[i].damage / 100.0f : 0.0f;
    equippedArmor.armorValue -= reduction;
    if (equippedArmor.armorValue < 0 && equippedArmor.item != SpikyArmor) {
        equippedArmor.armorValue = 0;
        snprintf(equippedArmor.description, sizeof(equippedArmor.description),
                 "%s", "Broken armor.");
    }
    equippedArmorValue = equippedArmor.armorValue;
}

void shootProjectile(float x, float y, float xDir, float yDir,
                     bool shotByPlayer, int shooterId) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (projectiles[i].active) continue;
        projectiles[i].x          = x;
        projectiles[i].y          = y;
        projectiles[i].dx         = xDir;
        projectiles[i].dy         = yDir;
        projectiles[i].damage     = shotByPlayer
            ? (float)(playerAttackDamage +
                      strengthRingsNumber * STRENGTH_RING_DAMAGE_BONUS +
                      weaknessRingsNumber * WEAKNESS_RING_DAMAGE_BONUS)
            : (float)PROJECTILE_DAMAGE_ENEMY;
        projectiles[i].speed      = shotByPlayer
            ? PROJECTILE_SPEED_PLAYER : PROJECTILE_SPEED_ENEMY;
        projectiles[i].active     = true;
        projectiles[i].shotByPlayer = shotByPlayer;
        projectiles[i].shooterId  = shooterId;
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Rendering
// ─────────────────────────────────────────────────────────────────────────────
void renderEnemies() {
    int ptx = predictXtile(playerX);
    int pty = predictYtile(playerY);
    int visRadSq = 200 - (invisibleRingsNumber * 50);

    for (int i = 0; i < MAX_ENEMIES; i++) {
        bool isBoss = (strcmp(enemies[i].name, "boss") == 0);
        if (!isBoss && enemies[i].hp <= 0) continue;

        int etx = predictXtile(enemies[i].x);
        int ety = predictYtile(enemies[i].y);
        int dx  = etx - ptx, dy = ety - pty;
        if (dx * dx + dy * dy > visRadSq) continue;

        if (!seeAll && !isVisible(ptx, pty, etx, ety)) continue;

        float sx = (enemies[i].x - offsetX) * tileSize;
        float sy = (enemies[i].y - offsetY) * tileSize;
        if (sx < 0 || sy < 0 || sx >= SCREEN_WIDTH || sy >= SCREEN_HEIGHT) continue;

        if (strcmp(enemies[i].name, "succubus") == 0) {
            const unsigned char* spr = (playerX > enemies[i].x)
                                       ? enemies[i].sprite : succubusIdleSpriteFlipped;
            display.drawBitmap((int)sx, (int)sy, spr, 8, 8, 15);
        } else {
            int siz = isBoss ? 16 : 8;
            display.drawBitmap((int)sx, (int)sy, enemies[i].sprite, siz, siz, 15);
        }
    }
}

void renderDamsel() {
    if (damsel[0].beingCarried || !damsel[0].active) return;

    int ptx = predictXtile(playerX),  pty = predictYtile(playerY);
    int dtx = predictXtile(damsel[0].x), dty = predictYtile(damsel[0].y);
    int dx = dtx - ptx, dy = dty - pty;
    if (dx * dx + dy * dy > 100) return;
    if (!isVisible(ptx, pty, dtx, dty)) return;

    float sx = (damsel[0].x - offsetX) * tileSize;
    float sy = (damsel[0].y - offsetY) * tileSize;
    if (sx < 0 || sy < 0 || sx >= SCREEN_WIDTH || sy >= SCREEN_HEIGHT) return;

    const unsigned char* spr = damsel[0].dead ? damselSpriteDead : damselSprite;
    display.drawBitmap((int)sx, (int)sy, spr, 8, 8, 15);
}

void renderProjectiles() {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!projectiles[i].active) continue;
        float sx = (projectiles[i].x - offsetX) * tileSize + tileSize / 2.0f;
        float sy = (projectiles[i].y - offsetY) * tileSize + tileSize / 2.0f;
        display.fillCircle((int)sx, (int)sy, 1, 15);
    }
}

void spawnParticles(float x, float y, int count, float speed, bool isLarge) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < MAX_PARTICLES; j++) {
            if (particles[j].active) continue;
            float angle      = random(0, 628) / 100.0f;
            particles[j].x          = x;
            particles[j].y          = y;
            particles[j].vx         = cosf(angle) * speed;
            particles[j].vy         = sinf(angle) * speed;
            particles[j].maxLifetime = isLarge ? random(8, 15) : random(5, 10);
            particles[j].lifetime   = particles[j].maxLifetime;
            particles[j].active     = true;
            break;
        }
    }
}

void updateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        particles[i].x  += particles[i].vx;
        particles[i].y  += particles[i].vy;
        particles[i].vx *= 0.92f;
        particles[i].vy *= 0.92f;
        if (--particles[i].lifetime <= 0) particles[i].active = false;
    }
}

void renderParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        float sx = (particles[i].x - offsetX) * tileSize;
        float sy = (particles[i].y - offsetY) * tileSize;
        if (sx < -tileSize || sx >= SCREEN_WIDTH  + tileSize ||
            sy < -tileSize || sy >= SCREEN_HEIGHT + tileSize) continue;
        if (particles[i].maxLifetime >= 20)
            display.fillRect((int)sx, (int)sy, 2, 2, 15);
        else
            display.fillCircle((int)sx + 1, (int)sy + 1, 1, 15);
    }
}