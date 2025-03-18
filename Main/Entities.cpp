#include "Entities.h"
#include "Sprites.h"
#include "HelperFunctions.h"
#include "Dungeon.h"
#include "Player.h"

#include <cmath>

// --- A* pathfinding implementation ---
// Using cost values: straight = 10, diagonal = 14, and Manhattan heuristic.
#define INF 999999
#define COST_STRAIGHT 10
#define COST_DIAGONAL 14

Damsel damsel[1];
Enemy enemies[maxEnemies];
Projectile projectiles[maxProjectiles];
int levelOfDamselDeath = -4;

struct Coord {
  int x, y;
};

Dialogue damselAnnoyingDialogue[] = {
  {"Why are you even down here, anyway?", 500},
  {"Don't die.", 400},
  {"I hate this place!", 300, "annoying"},
  {"Why do wizards wear that stupid hat?", 500, "annoying"},
  {"Hurry up, will you?", 300},
  {"You expect a thanks, don't you? Not gonna happen.", 500, "annoying"},
  {"Does this place ever end?", 350},
  {"Just saying, I don't like you.", 500, "annoying"},
  {"When was the last time you showered?", 500, "annoying"},
  {"I could probably get out on my own.", 400}
};

Dialogue damselPassiveDialogue[] = {
  {"Maybe you're not as dumb as I thought.", 500},
  {"Maybe you know where you're going after all...", 500},
  {"Sorry if I said anything hurtful.", 500, "alone"},
  {"Please don't die.", 300, "alone"},
  {"I probably couldn't get out on my own.", 500},
  {"Thanks for rescuing me, anyway.", 400, "alone"},
  {"Well, at least I'm not alone anymore.", 500}
};

Dialogue damselGoodDialogue[] = {
  {"Actually, I do kind of like you.", 400, "alone"},
  {"I'm sorry if I was annoying you.", 400},
  {"Do you mind... carrying me? (Hold B next to me)", 400, "alone"},
  {"Does that staff ever run out?", 350},
  {"I'm glad I'm with you.", 300},
  {"I don't want to die.", 300, "annoying"},
  {"Do you think we're almost at the end?", 500}
};

Dialogue damselCarryDialogue[] = {
  {"You're actually kind of strong...", 450},
  {"Can I stay in your arms for a bit?", 450},
  {"You can put me down if you want.", 500},
  {"I kind of like it here...", 300},
  {"Can we... never mind.", 300},
  {"I don't want to be in that cell again!", 500},
  {"Can I stay with you after we escape?", 500}
};

bool computePath(int startX, int startY, int goalX, int goalY, PathNode* path, int &pathLength, int maxPathNodes = 32) {
  int g[mapHeight][mapWidth];
  int f[mapHeight][mapWidth];
  bool closed[mapHeight][mapWidth];
  bool inOpen[mapHeight][mapWidth];
  int parentX[mapHeight][mapWidth];
  int parentY[mapHeight][mapWidth];

  // Initialize arrays
  for (int y = 0; y < mapHeight; y++) {
    for (int x = 0; x < mapWidth; x++) {
      g[y][x] = INF;
      f[y][x] = INF;
      closed[y][x] = false;
      inOpen[y][x] = false;
      parentX[y][x] = -1;
      parentY[y][x] = -1;
    }
  }

  // Open list as a simple array
  Coord openList[mapWidth * mapHeight];
  int openCount = 0;

  g[startY][startX] = 0;
  int h = (abs(goalX - startX) + abs(goalY - startY)) * COST_STRAIGHT;
  f[startY][startX] = h;
  openList[openCount++] = {startX, startY};
  inOpen[startY][startX] = true;

  // Offsets for 8 neighbors (diagonals included)
  int dx[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
  int dy[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };

  bool pathFound = false;
  int currentX, currentY;

  while (openCount > 0) {
    // Find node with lowest f value
    int lowestIndex = 0;
    int lowestF = f[ openList[0].y ][ openList[0].x ];
    for (int i = 1; i < openCount; i++) {
      int curF = f[ openList[i].y ][ openList[i].x ];
      if (curF < lowestF) {
        lowestF = curF;
        lowestIndex = i;
      }
    }
    Coord current = openList[lowestIndex];
    currentX = current.x;
    currentY = current.y;

    // Remove current from open list
    openList[lowestIndex] = openList[openCount - 1];
    openCount--;
    inOpen[currentY][currentX] = false;
    closed[currentY][currentX] = true;

    // Check if goal is reached
    if (currentX == goalX && currentY == goalY) {
      pathFound = true;
      break;
    }

    // Process neighbors
    for (int i = 0; i < 8; i++) {
      int nx = currentX + dx[i];
      int ny = currentY + dy[i];

      if (nx < 0 || nx >= mapWidth || ny < 0 || ny >= mapHeight) continue;
      if (!isWalkable(nx, ny)) continue;
      if (closed[ny][nx]) continue;

      // Determine cost: diagonal if both offsets nonzero
      int moveCost = (dx[i] != 0 && dy[i] != 0) ? COST_DIAGONAL : COST_STRAIGHT;
      int tentativeG = g[currentY][currentX] + moveCost;
      if (!inOpen[ny][nx] || tentativeG < g[ny][nx]) {
        parentX[ny][nx] = currentX;
        parentY[ny][nx] = currentY;
        g[ny][nx] = tentativeG;
        int heuristic = (abs(goalX - nx) + abs(goalY - ny)) * COST_STRAIGHT;
        f[ny][nx] = tentativeG + heuristic;
        if (!inOpen[ny][nx]) {
          openList[openCount++] = {nx, ny};
          inOpen[ny][nx] = true;
        }
      }
    }
  }

  if (!pathFound) {
    return false;
  }

  // Reconstruct the path (from goal to start)
  PathNode tempPath[mapWidth * mapHeight];
  int tempLength = 0;
  int cx = goalX, cy = goalY;
  while (!(cx == startX && cy == startY)) {
    tempPath[tempLength].x = cx;
    tempPath[tempLength].y = cy;
    tempLength++;
    int px = parentX[cy][cx];
    int py = parentY[cy][cx];
    cx = px;
    cy = py;
    if (tempLength >= mapWidth * mapHeight) break;
  }
  // Include the start node
  tempPath[tempLength].x = startX;
  tempPath[tempLength].y = startY;
  tempLength++;

  // Reverse the path so that it goes from start to goal;
  // only copy up to maxPathNodes nodes (e.g. 32)
  int nodesToCopy = (tempLength < maxPathNodes) ? tempLength : maxPathNodes;
  for (int i = 0; i < nodesToCopy; i++) {
    path[i] = tempPath[tempLength - 1 - i];
  }
  pathLength = nodesToCopy;
  return true;
}

int damselMoveDelay = 0;
void updateDamsel() {
  if (!damsel[0].dead) {
    damselMoveDelay++;
  } else {
    damsel[0].followingPlayer = false;
    return;
  }

  float destinationX;
  float destinationY;

  destinationX = playerDX == 1 ? playerX - 1 : playerDX == -1 ? playerX + 1 : playerX;
  destinationY = playerDY == 1 ? playerY - 1 : playerDY == -1 ? playerY + 1 : playerY;

  // Calculate distance to player
  int dx = round(destinationX) - round(damsel[0].x);
  int dy = round(destinationY) - round(damsel[0].y);
  int distanceSquared = dx * dx + dy * dy;

  if (carryingDamsel) {
    damsel[0].x = playerX;
    damsel[0].y = playerY;
    return;
  }

  // Check if the damsel should follow the player
  if (distanceSquared <= 25 + (damsel[0].levelOfLove*2)) { // Follow if within 5 tiles (distance^2 = 25) + the love level
    damsel[0].followingPlayer = true;
    damsel[0].speed = 0.3;
  } else {
    damsel[0].followingPlayer = false;
    damsel[0].speed = 0.1;
  }

  if (!damsel[0].followingPlayer) {
    // Random wandering
    if (damselMoveDelay >= 30) {
      int dir = random(0, 4);
      float nx = damsel[0].x + (dir == 0 ? damsel[0].speed : dir == 1 ? -damsel[0].speed : 0);
      float ny = damsel[0].y + (dir == 2 ? damsel[0].speed : dir == 3 ? -damsel[0].speed : 0);

      damselSprite = dir == 0 ? damselSpriteRight : dir == 1 ? damselSpriteLeft : damselSprite;

      // Check bounds and avoid walls
      if (!checkSpriteCollisionWithTileX(nx, damsel[0].x, ny)) {
        damsel[0].x = nx;
      }
      if (!checkSpriteCollisionWithTileY(ny, damsel[0].y, nx)) {
        damsel[0].y = ny;
      }
      damselMoveDelay = 0;
    }
  } else {
    if (damsel[0].levelOfLove == 0) {
      showDialogue = true;
      currentDamselPortrait = damselPortraitNormal;
      dialogueTimeLength = 500;
      currentDialogue = "Hey! I shall follow you, please get me out of here.";
      damsel[0].levelOfLove = 1;
    }
    if (damselGotTaken && damselSayThanksForRescue) {
      showDialogue = true;
      currentDamselPortrait = damselPortraitAlone;
      dialogueTimeLength = 400;
      currentDialogue = "He- wasn't gentle...";
      damselSayThanksForRescue = false;
    }

    // Following the player
    if (damselMoveDelay >= 3) {
      float moveX = (dx > 0 ? 1 : dx < 0 ? -1 : 0);
      float moveY = (dy > 0 ? 1 : dy < 0 ? -1 : 0);

      damselSprite = moveX == 1 ? damselHopefullSpriteRight : moveX == -1 ? damselHopefullSpriteLeft : damselSprite;

      // Normalize movement vector
      float magnitude = sqrt(moveX * moveX + moveY * moveY);
      if (magnitude > 0) {
        moveX /= magnitude;
        moveY /= magnitude;
      }

      // Attempt to move diagonally first
      float nx = damsel[0].x + moveX * damsel[0].speed;
      float ny = damsel[0].y + moveY * damsel[0].speed;

      bool xValid = !checkSpriteCollisionWithTileX(nx, damsel[0].x, damsel[0].y);
      bool yValid = !checkSpriteCollisionWithTileY(ny, damsel[0].y, damsel[0].x);

      if (xValid && yValid) {
        // Move diagonally
        damsel[0].x = nx;
        damsel[0].y = ny;
      } else if (xValid) {
        // Slide along X
        damsel[0].x = nx;
      } else if (yValid) {
        // Slide along Y
        damsel[0].y = ny;
      } else {
        // Both directions blocked, try wall sliding
        float slideX = damsel[0].x + moveX * damsel[0].speed;
        float slideY = damsel[0].y;

        if (!checkSpriteCollisionWithTileX(slideX, damsel[0].x, damsel[0].y)) {
          damsel[0].x = slideX;
        } else if (!checkSpriteCollisionWithTileY(slideY, damsel[0].y, damsel[0].x)) {
          damsel[0].y = slideY;
        }
      }

      damselMoveDelay = 0;
    }
  }
}

int atkDelayCounter = 0;
void updateEnemies() {
  atkDelayCounter += 1;
  for (int i = 0; i < maxEnemies; i++) {
    if (enemies[i].hp <= 0) continue; // Skip dead enemies

    // Work in grid space (rounding enemy and player positions)
    int enemyGridX = round(enemies[i].x);
    int enemyGridY = round(enemies[i].y);
    int playerGridX = round(playerX);
    int playerGridY = round(playerY);

    int dx = playerGridX - enemyGridX;
    int dy = playerGridY - enemyGridY;
    int distanceSquared = dx * dx + dy * dy;

    // Determine if the enemy should chase (within 5 tiles)
    if (distanceSquared <= 25) {
      enemies[i].chasingPlayer = true;
    } else {
      enemies[i].chasingPlayer = false;
    }

    if (enemies[i].chasingPlayer) {
      // When chasing, compute a dynamic path using A*
      PathNode dynamicPath[32];
      int dynamicPathLength = 0;
      bool found = computePath(enemyGridX, enemyGridY, playerGridX, playerGridY, dynamicPath, dynamicPathLength);
      if (found && dynamicPathLength > 1) {
        // The first node is the enemy's current cell; move toward the next cell
        int nextX = dynamicPath[1].x;
        int nextY = dynamicPath[1].y;
        float targetX = nextX;
        float targetY = nextY;
        float moveX = targetX - enemies[i].x;
        float moveY = targetY - enemies[i].y;
        float magnitude = sqrt(moveX * moveX + moveY * moveY);
        if (magnitude > 0) {
          moveX = (moveX / magnitude) * enemies[i].moveAmount;
          moveY = (moveY / magnitude) * enemies[i].moveAmount;
        }
        float nx = enemies[i].x + moveX;
        float ny = enemies[i].y + moveY;
        bool xValid = !checkSpriteCollisionWithTileX(nx, enemies[i].x, enemies[i].y);
        bool yValid = !checkSpriteCollisionWithTileY(ny, enemies[i].y, enemies[i].x);
        if (xValid && yValid) {
          enemies[i].x = nx;
          enemies[i].y = ny;
        } else if (xValid) {
          enemies[i].x = nx;
        } else if (yValid) {
          enemies[i].y = ny;
        }
      }
    } else {
      // When not chasing, follow a precomputed wander path
      if (!enemies[i].hasWanderPath) {
        // Compute a new wander path: choose a random destination near the enemy
        int startX = enemyGridX;
        int startY = enemyGridY;
        int destX = startX + random(-5, 6);
        int destY = startY + random(-5, 6);
        // Clamp destination to the dungeon boundaries
        if (destX < 0) destX = 0;
        if (destX >= mapWidth) destX = mapWidth - 1;
        if (destY < 0) destY = 0;
        if (destY >= mapHeight) destY = mapHeight - 1;
        if (isWalkable(destX, destY)) {
          if (computePath(startX, startY, destX, destY, enemies[i].wanderPath, enemies[i].pathLength)) {
            enemies[i].currentPathIndex = 0;
            enemies[i].hasWanderPath = true;
          }
        }
      } else {
        // Follow the wander path nodes one by one
        if (enemies[i].currentPathIndex < enemies[i].pathLength) {
          int nextX = enemies[i].wanderPath[enemies[i].currentPathIndex].x;
          int nextY = enemies[i].wanderPath[enemies[i].currentPathIndex].y;
          // If the enemy is on the target cell, advance the index
          if (enemyGridX == nextX && enemyGridY == nextY) {
            enemies[i].currentPathIndex++;
            if (enemies[i].currentPathIndex >= enemies[i].pathLength) {
              enemies[i].hasWanderPath = false;
            }
          } else {
            float targetX = nextX;
            float targetY = nextY;
            float moveX = targetX - enemies[i].x;
            float moveY = targetY - enemies[i].y;
            float magnitude = sqrt(moveX * moveX + moveY * moveY);
            if (magnitude > 0) {
              moveX = (moveX / magnitude) * enemies[i].moveAmount;
              moveY = (moveY / magnitude) * enemies[i].moveAmount;
            }
            float nx = enemies[i].x + moveX;
            float ny = enemies[i].y + moveY;
            bool xValid = !checkSpriteCollisionWithTileX(nx, enemies[i].x, enemies[i].y);
            bool yValid = !checkSpriteCollisionWithTileY(ny, enemies[i].y, enemies[i].x);
            if (xValid && yValid) {
              enemies[i].x = nx;
              enemies[i].y = ny;
            } else if (xValid) {
              enemies[i].x = nx;
            } else if (yValid) {
              enemies[i].y = ny;
            }
          }
        } else {
          // If the wander path is finished, clear it to compute a new one later.
          enemies[i].hasWanderPath = false;
        }
      }
    }

    // Existing collision with player logic remains unchanged:
    if (checkSpriteCollisionWithSprite(playerX, playerY, enemies[i].x, enemies[i].y)) {
      if (enemies[i].name == "teleporter") {
        int newX, newY;
        do {
          newX = random(0, mapWidth);
          newY = random(0, mapHeight);
        } while (dungeonMap[newY][newX] != Floor);
        playerX = newX;
        playerY = newY;
      } else {
        if (atkDelayCounter >= enemies[i].attackDelay) {
          playerHP -= enemies[i].damage;
          atkDelayCounter = 0;
        }
        if (playerHP <= 0) {
          deathCause = enemies[i].name;
        }
      }
    }
  }
}

void updateProjectiles() {
  for (int i = 0; i < maxProjectiles; i++) {
    if (projectiles[i].active) {
      projectiles[i].x += projectiles[i].dx * projectiles[i].speed;
      projectiles[i].y += projectiles[i].dy * projectiles[i].speed;

      int projectileTileX = predictXtile(projectiles[i].x);
      int projectileTileY = predictYtile(projectiles[i].y);

      // Check for collisions with walls or out-of-bounds
      if (dungeonMap[projectileTileY][projectileTileX] == Wall || dungeonMap[projectileTileY][projectileTileX] == Bars || projectiles[i].x < 0 || projectiles[i].y < 0 || projectiles[i].x > SCREEN_WIDTH || projectiles[i].y > SCREEN_HEIGHT || projectiles[i].speed <= 0 || (projectiles[i].dx == 0 && projectiles[i].dy == 0)) {
        projectiles[i].active = false; // Deactivate the bullet
      }

      // Check for collisions with enemies
      for (int j = 0; j < maxEnemies; j++) {
        if (checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y, enemies[j].x, enemies[j].y) && enemies[j].hp > 0) {
          enemies[j].hp -= projectiles[i].damage;    // Reduce enemy health
          if (enemies[j].hp <= 0 && projectiles[i].active == true) {
            kills += 1;
          }
          projectiles[i].active = false; // Deactivate the bullet
        } else if (!damsel[0].dead && checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y, damsel[0].x, damsel[0].y)) {
          levelOfDamselDeath = dungeon;
          damsel[0].dead = true;
          showDialogue = true;
          currentDamselPortrait = damselPortraitDying;
          dialogueTimeLength = 200;
          currentDialogue = "Ugh-!";
          damsel[0].active = false;
          projectiles[i].active = false;
        }
      }
    }
  }
}

void moveDamselToPos(float posX, float posY) {
  damsel[0].x = posX;
  damsel[0].y = posY;
}

void shootProjectile(float xDir, float yDir) {

  for (int i = 0; i < maxProjectiles; i++) {
    if (!projectiles[i].active) {
      projectiles[i].x = playerX;
      projectiles[i].y = playerY;
      projectiles[i].dx = xDir;  // Set direction based on player's facing direction
      projectiles[i].dy = yDir;
      projectiles[i].damage = 10;
      projectiles[i].speed = 0.5;
      projectiles[i].active = true;
      break;
    }
  }
}

void renderEnemies() {
  int playerTileX = predictXtile(playerX);
  int playerTileY = predictYtile(playerY);

  for (int i = 0; i < maxEnemies; i++) {
    if (enemies[i].hp > 0) {
      int enemyTileX = predictXtile(enemies[i].x);
      int enemyTileY = predictYtile(enemies[i].y);

      // Distance check (10 tiles max)
      int dx = enemyTileX - playerTileX;
      int dy = enemyTileY - playerTileY;
      int distSq = dx*dx + dy*dy;
      if (distSq > 200) continue;

      // Line-of-sight check
      if (isVisible(playerTileX, playerTileY, enemyTileX, enemyTileY)) {
        float screenX = (enemies[i].x - offsetX) * tileSize;
        float screenY = (enemies[i].y - offsetY) * tileSize;
        if (screenX >= 0 && screenY >= 0 && screenX < SCREEN_WIDTH && screenY < SCREEN_HEIGHT) {
          display.drawBitmap(screenX, screenY, enemies[i].name == "blob" ? blobSprite : enemies[i].name == "teleporter" ? teleporterSprite : wallSprite, 8, 8, 15);
        }
      }
    }
  }
}

void renderDamsel() {
  if (carryingDamsel) return;

  int playerTileX = predictXtile(playerX);
  int playerTileY = predictYtile(playerY);
  int damselTileX = predictXtile(damsel[0].x);
  int damselTileY = predictYtile(damsel[0].y);

  // Distance check (10 tiles max)
  int dx = damselTileX - playerTileX;
  int dy = damselTileY - playerTileY;
  int distSq = dx * dx + dy * dy;
  if (distSq > 100) return; // Skip rendering if too far away

  if (isVisible(playerTileX, playerTileY, damselTileX, damselTileY)) {
    if (damsel[0].active && !damsel[0].dead) {
      float screenX = (damsel[0].x - offsetX) * tileSize;
      float screenY = (damsel[0].y - offsetY) * tileSize;
      if (screenX >= 0 && screenY >= 0 && screenX < SCREEN_WIDTH && screenY < SCREEN_HEIGHT) {
        display.drawBitmap(screenX, screenY, damselSprite, 8, 8, 15);
      }
    } else {
      float screenX = (damsel[0].x - offsetX) * tileSize;
      float screenY = (damsel[0].y - offsetY) * tileSize;
      if (screenX >= 0 && screenY >= 0 && screenX < SCREEN_WIDTH && screenY < SCREEN_HEIGHT) {
        display.drawBitmap(screenX, screenY, damselSpriteDead, 8, 8, 15);
      }
    }
  }
}

void renderProjectiles() {
    for (int i = 0; i < maxProjectiles; i++) {
        if (projectiles[i].active) {
          float screenX = (projectiles[i].x - offsetX) * tileSize + tileSize/2;
          float screenY = (projectiles[i].y - offsetY) * tileSize + tileSize/2;
          display.fillCircle(screenX, screenY, 1, 15);
        }
    }
}