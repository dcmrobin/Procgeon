#include "Entities.h"
#include "Sprites.h"
#include "HelperFunctions.h"
#include "Dungeon.h"
#include "Player.h"
#include "GameAudio.h"

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
float clockX = -10000;
float clockY = -10000;

struct Coord {
  int x, y;
};

BossStates bossState = Idle;
int bossStateTimer = 0;

Dialogue damselAnnoyingDialogue[] = {
  {"Why are you even down here, anyway?", 400},
  {"Don't die.", 400},
  {"I hate this place!", 300, "annoying"},
  {"Why do wizards wear that stupid hat?", 400, "annoying"},
  {"Hurry up, will you?", 300},
  {"You expect a thanks, don't you? Not gonna happen.", 400, "annoying"},
  {"Does this place ever end?", 350},
  {"Just saying, I don't like you.", 400, "annoying"},
  {"When was the last time you showered?", 400, "annoying"},
  {"I could probably get out on my own.", 400}
};

Dialogue damselPassiveDialogue[] = {
  {"Maybe you're not as dumb as I thought.", 400},
  {"Maybe you know where you're going after all...", 400},
  {"Sorry if I said anything hurtful.", 400, "alone"},
  {"Please don't die.", 300, "alone"},
  {"I probably couldn't get out on my own.", 400},
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
  {"Do you think we're almost at the end?", 400}
};

Dialogue damselCarryDialogue[] = {
  {"You're actually kind of strong...", 450},
  {"Can I stay in your arms for a bit?", 450},
  {"You can put me down if you want.", 500},
  {"I kind of like it here...", 300},
  {"Hm...", 300},
  {"I don't want to be in that cell again!", 500},
  {"Can I stay with you after we escape?", 500}
};

Dialogue ridiculeDialogue[] = {
  {"Is that the best you can do?", 400},
  {"I've seen slimes with more brains.", 400},
  {"Use your brain! Oh wait- you don't have one.", 400},
  {"Nice move genius.", 400},
  {"You call that magic?", 400},
  {"You're not the greatest at this.", 400},
  {"Wow. Just wow.", 400},
  {"Maybe potions just aren't your thing.", 400}
};

Dialogue glamourDialogue[] = {
  {"You're looking sharp today!", 400},
  {"That was some impressive magic!", 400},
  {"You make this look so easy.", 400},
  {"Is there anything you can't do?", 400},
  {"You're a true hero!", 400},
  {"Many aren't as skilled as you.", 400},
  {"Your bravery is very inspiring.", 400},
  {"Keep going, champion!", 400}
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

  if (nearSuccubus) {
    damsel[0].followingPlayer = false;
    destinationX = damsel[0].x;
    destinationY = damsel[0].y;// Damsel is frozen in fear
  }

  // Calculate distance to player
  int dx = round(destinationX) - round(damsel[0].x);
  int dy = round(destinationY) - round(damsel[0].y);
  int distanceSquared = dx * dx + dy * dy;

  if (damsel[0].beingCarried) {
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

  // Check if damsel just stopped following and say "Hey! Wait up!"
  // Only show this dialogue if the damsel is actually following the player and not in a cell
  // Also ensure no succubus is currently chasing the player
  bool succubusChasing = false;
  for (int i = 0; i < maxEnemies; i++) {
    if (enemies[i].hp > 0 && enemies[i].name == "succubus" && enemies[i].chasingPlayer && !enemies[i].isFriend) {
      succubusChasing = true;
      break;
    }
  }
  
  if (!damsel[0].completelyRescued && !nearSuccubus && !succubusChasing && damselWasFollowing && !damsel[0].followingPlayer && !damselSaidWaitUp && damselWaitUpTimer <= 0 && !damselGotTaken && damsel[0].active) {
    currentDamselPortrait = damselPortraitScared;
    dialogueTimeLength = 300;
    playRawSFX3D(17, damsel[0].x, damsel[0].y);
    currentDialogue = "Hey! Wait up!";
    showDialogue = true;
    damselSaidWaitUp = true;
    damselWaitUpTimer = 200; // Prevent spam for 200 frames
  }

  // Update tracking variables
  damselWasFollowing = damsel[0].followingPlayer;
  
  // Decrement timers
  if (damselWaitUpTimer > 0) {
    damselWaitUpTimer--;
  }
  
  // Reset the flag when timer expires
  if (damselWaitUpTimer <= 0) {
    damselSaidWaitUp = false;
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
      currentDamselPortrait = damselPortraitNormal;
      dialogueTimeLength = 400;
      playRawSFX(16);
      currentDialogue = "Hey! I shall follow you, please get me out of here.";
      showDialogue = true;
      damsel[0].levelOfLove = 1;
    }
    if (damselGotTaken && damselSayThanksForRescue && !succubusIsFriend) {
      playRawSFX(17);
      currentDamselPortrait = damselPortraitAlone;
      dialogueTimeLength = 400;
      currentDialogue = "He- wasn't gentle...";
      showDialogue = true;
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

int giveUpTimer = 0;
void updateEnemies() {
  // --- First pass: find the clock enemy and set clockX/clockY ---
  for (int i = 0; i < maxEnemies; i++) {
    if (enemies[i].hp > 0 && enemies[i].name == "clock") {
      clockX = enemies[i].x;
      clockY = enemies[i].y;
      break; // Only one clock enemy assumed
    }
  }

  // --- Second pass: set nearClock for all non-clock enemies ---
  for (int i = 0; i < maxEnemies; i++) {
    if (enemies[i].name == "boss") {
      continue; // Skip boss, its AI is handled in Main.ino in updateBossfight()
    }
    if (enemies[i].hp > 0 && enemies[i].name != "clock") {
      float clockDiffX = enemies[i].x - clockX;
      float clockDiffY = enemies[i].y - clockY;
      float clockDistanceSquared = clockDiffX * clockDiffX + clockDiffY * clockDiffY;
      float clockDistance = sqrt(clockDistanceSquared);
      if (clockDistance <= 5) {
        enemies[i].nearClock = true;
      } else {
        enemies[i].nearClock = false;
      }
    }
  }

  // --- Third pass: main update logic ---

  // Find closest jukebox to the player and set jukebox music volume accordingly
  {
    float closestDist = 1e9;
    int closestIndex = -1;
    for (int j = 0; j < maxEnemies; j++) {
      if (enemies[j].hp > 0 && enemies[j].name == "jukebox") {
        float dx = enemies[j].x - playerX;
        float dy = enemies[j].y - playerY;
        float dist = sqrt(dx*dx + dy*dy);
        if (dist < closestDist) {
          closestDist = dist;
          closestIndex = j;
        }
      }
    }
    float vol = 0.0f;
    if (closestIndex != -1) {
      // Map distance to volume: within maxRadius tiles full volume, beyond zero
      const float maxRadius = 12.0f;
      if (closestDist < maxRadius) {
        vol = 1.0f - (closestDist / maxRadius);
        if (vol < 0.0f) vol = 0.0f;
      }
    }
    setJukeboxVolume(vol);

    // Increase ambient noise proportional to jukebox volume so enemies are attracted
    int noiseFromJukebox = (int)round(vol * 8.0f); // 0..8
    if (ambientNoiseLevel < noiseFromJukebox) ambientNoiseLevel = noiseFromJukebox;
  }

  for (int i = 0; i < maxEnemies; i++) {
    if (enemies[i].hp <= 0) continue; // Skip dead enemies
    
    // Check if enemy is stuck in a wall and unstuck them
    unstuckEnemy(enemies[i]);
    
    if (enemies[i].name == "boss") {
      // Skip boss AI but allow friendly enemies to attack it
      if (enemies[i].hp > 0) {
        // Check if any friendly enemies are near the boss and can attack it
        for (int j = 0; j < maxEnemies; j++) {
          if (j != i && enemies[j].hp > 0 && enemies[j].isFriend) {
            float dist = sqrt((enemies[i].x - enemies[j].x)*(enemies[i].x - enemies[j].x) + (enemies[i].y - enemies[j].y)*(enemies[i].y - enemies[j].y));
            if (dist < 0.5) {
              // Friendly enemy is touching boss, allow attack
              if (enemies[j].attackDelayCounter >= enemies[j].attackDelay) {
                enemies[i].hp -= enemies[j].damage;
                if (enemies[i].hp <= 0) {
                  kills += 1;
                }
                playRawSFX3D(23, enemies[i].x, enemies[i].y); // Play hit sound
                enemies[j].attackDelayCounter = 0;
              } else {
                enemies[j].attackDelayCounter++;
              }
            }
          }
        }
      }
      continue; // Skip boss AI, it's handled in Main.ino in updateBossfight()
    }
    if (!playerActed && !enemies[i].nearClock && enemies[i].name != "clock") { continue; }

    // Calculate the vector from the player to the enemy.
    float diffX = enemies[i].x - playerX;
    float diffY = enemies[i].y - playerY;
    int distanceSquared = diffX * diffX + diffY * diffY;
    float distance = sqrt(distanceSquared);

    if (enemies[i].name == "clock") {
      // clockX/clockY already set above
      if (distance <= 5) {
        playerNearClockEnemy = true;
      } else {
        playerNearClockEnemy = false;
      }
    }

    // Only consider enemies within a certain range (e.g. within 8 tiles)
    if (distance > 0 && distanceSquared < 64) // 64 = 8^2
      {
      // Assuming playerDX and playerDY are normalized (or nearly so),
      // compute the cosine of the angle between the player's facing and the enemy's direction.
      float dot = diffX * playerDX + diffY * playerDY;
      float cosAngle = dot / distance; // since (playerDX,playerDY) is roughly unit length
      
      // If cosAngle is very high (close to 1), the enemy lies nearly directly ahead.
      const float avoidanceThreshold = 0.95; // adjust threshold as needed
      if (cosAngle > avoidanceThreshold) {
        // Check if there's space to dodge by looking at adjacent tiles
        int enemyGridX = round(enemies[i].x);
        int enemyGridY = round(enemies[i].y);
        
        // Check if we're in a corridor (limited space)
        bool inCorridor = false;
        int freeDirections = 0;
        
        // Check all 8 directions for walkable space
        for (int dx = -1; dx <= 1; dx++) {
          for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue; // Skip the center tile
            
            int checkX = enemyGridX + dx;
            int checkY = enemyGridY + dy;
            
            if (checkX >= 0 && checkX < mapWidth && checkY >= 0 && checkY < mapHeight) {
              if (dungeonMap[checkY][checkX] == Floor) {
                freeDirections++;
              }
            }
          }
        }
        
        // If we have less than 3 free directions, we're in a corridor
        inCorridor = (freeDirections < 3);
        
        if (!inCorridor) {
          // Determine which dodge direction gets the enemy out of line-of-sight faster
          // by choosing the perpendicular that moves away from the player's position
          float perpLeft_x = -playerDY;
          float perpLeft_y = playerDX;
          float perpRight_x = playerDY;
          float perpRight_y = -playerDX;
          
          // Calculate dot product of each perpendicular with the direction away from player
          float awayFromPlayerX = enemies[i].x - playerX;
          float awayFromPlayerY = enemies[i].y - playerY;
          
          float dotLeft = perpLeft_x * awayFromPlayerX + perpLeft_y * awayFromPlayerY;
          float dotRight = perpRight_x * awayFromPlayerX + perpRight_y * awayFromPlayerY;
          
          // Choose the perpendicular with the higher dot product (points more away from player)
          int chosenDodgeDir = (dotLeft >= dotRight) ? 1 : -1;
          
          // Use the chosen direction
          float avoidX = chosenDodgeDir * -playerDY;
          float avoidY = chosenDodgeDir * playerDX;
          
          // Also calculate the direction toward the player
          float towardPlayerX = -diffX / distance;
          float towardPlayerY = -diffY / distance;
          
          // Combine dodge and approach movements (70% dodge, 30% approach)
          float combinedX = (avoidX * 0.7f) + (towardPlayerX * 0.3f);
          float combinedY = (avoidY * 0.7f) + (towardPlayerY * 0.3f);
          
          // Normalize the combined vector
          float combinedMagnitude = sqrt(combinedX * combinedX + combinedY * combinedY);
          if (combinedMagnitude > 0) {
            combinedX /= combinedMagnitude;
            combinedY /= combinedMagnitude;
          }
          
          // Multiply by enemy's moveAmount
          float avoidSpeed = enemies[i].moveAmount;
          float newX = enemies[i].x + combinedX * avoidSpeed;
          float newY = enemies[i].y + combinedY * avoidSpeed;
          
          // Check for collisions before moving
          bool xValid = !checkSpriteCollisionWithTileX(newX, enemies[i].x, enemies[i].y);
          bool yValid = !checkSpriteCollisionWithTileY(newY, enemies[i].y, enemies[i].x);
          if (xValid && yValid) {
            enemies[i].x = newX;
            enemies[i].y = newY;
          } else if (xValid) {
            enemies[i].x = newX;
          } else if (yValid) {
            enemies[i].y = newY;
          }
          // Skip the rest of the enemy update for this enemy so that it prioritizes avoiding the shot.
          continue;
        }
        // If in corridor, fall through to normal movement
      }
    }

    // Existing grid-space logic for chase/wander behavior:
    int enemyGridX = round(enemies[i].x);
    int enemyGridY = round(enemies[i].y);
    int playerGridX = round(playerX);
    int playerGridY = round(playerY);
    int dx = playerGridX - enemyGridX;
    int dy = playerGridY - enemyGridY;
    int gridDistanceSquared = dx * dx + dy * dy;
    
    // Different behavior for friendly vs hostile enemies
    if (enemies[i].isFriend) {
      // Friendly enemies try to stay near the player
      if (gridDistanceSquared > 4) { // If more than 2 tiles away
        enemies[i].chasingPlayer = true;
      } else {
        enemies[i].chasingPlayer = false;
        // Look for nearby hostile enemies to attack
        for (int j = 0; j < maxEnemies; j++) {
          if (j != i && enemies[j].hp > 0 && !enemies[j].isFriend) {
            int targetDx = round(enemies[j].x) - enemyGridX;
            int targetDy = round(enemies[j].y) - enemyGridY;
            int targetDistSq = targetDx * targetDx + targetDy * targetDy;
            if (targetDistSq <= 25) { // Within 5 tiles
              enemies[i].chasingPlayer = true;
              // Override player position with enemy position for chasing
              playerGridX = round(enemies[j].x);
              playerGridY = round(enemies[j].y);
              break;
            }
          }
        }
      }
    } else {
      // Regular hostile enemy behavior
      if (gridDistanceSquared <= (25 + (ambientNoiseLevel))) { // Chase if within 5 tiles + noise factor
        enemies[i].chasingPlayer = true;
      } else {
        if (enemies[i].chasingPlayer) {
          giveUpTimer++;
        }
        if (giveUpTimer >= 600) {
          enemies[i].chasingPlayer = false;
          giveUpTimer = 0;
        }
      }
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
      if (enemies[i].name == "succubus") {
        continue; // Succubi do not wander when not chasing
      }

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
      }
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

    // Shooter enemies shoot projectiles when chasing the player
    if (enemies[i].name == "shooter" && enemies[i].chasingPlayer) {
      // Add a cooldown to prevent constant shooting
      static int shooterCooldown[maxEnemies] = {0};
      if (shooterCooldown[i] <= 0) {
        // Calculate direction to player
        float dirX = playerX - enemies[i].x;
        float dirY = playerY - enemies[i].y;
        float distance = sqrt(dirX * dirX + dirY * dirY);
        
        if (distance > 0) {
          // Normalize direction
          dirX /= distance;
          dirY /= distance;
          
          // Shoot projectile from enemy position towards player
          shootProjectile(enemies[i].x, enemies[i].y, dirX, dirY, false, i);
          
          // Set cooldown (adjust timing as needed)
          shooterCooldown[i] = 120; // 2 seconds at 60 FPS
        }
      } else {
        shooterCooldown[i]--;
      }
    }

    bool isAttacking = false;
    bool hasAttacked = false;
    
    // Handle collisions differently for friendly and hostile enemies
    if (checkSpriteCollisionWithSprite(playerX, playerY, enemies[i].x, enemies[i].y)) {
      if (enemies[i].name == "teleporter") {
        if (equippedArmor.item != MagicRobe) { // Only teleport if not wearing Magic Robe
          playRawSFX(14);
          int newX, newY;
          do {
            newX = random(0, mapWidth);
            newY = random(0, mapHeight);
          } while (dungeonMap[newY][newX] != Floor);
          playerX = newX;
          playerY = newY;
        }
      } else if (!enemies[i].isFriend) { // Only hostile enemies damage the player
        isAttacking = true;
        if (enemies[i].attackDelayCounter >= enemies[i].attackDelay) {
          int damage = enemies[i].damage - round(equippedArmorValue);
          reduceArmorDurability(i);
          if (damage < 0) damage = 0;
          if (damage > 0) {
            playerHP -= damage;
            triggerScreenShake(2, 1);
            playRawSFX(0);
            checkIfDeadFrom(enemies[i].name);
          }
          hasAttacked = true;
        }
      }
    }
    
    // Repel enemies from each other and handle combat between friendly and hostile enemies
    for (int j = 0; j < maxEnemies; j++) {
      if (i != j && enemies[j].hp > 0) {
        float dist = sqrt((enemies[i].x - enemies[j].x)*(enemies[i].x - enemies[j].x) + (enemies[i].y - enemies[j].y)*(enemies[i].y - enemies[j].y));
        if (dist < 0.5) {
          float dx = enemies[i].x - enemies[j].x;
          float dy = enemies[i].y - enemies[j].y;
          float mag = sqrt(dx*dx + dy*dy) + 0.01;
          enemies[i].x += (dx/mag) * 0.05;
          enemies[i].y += (dy/mag) * 0.05;
          
          // Handle combat between friendly and hostile enemies
          if (enemies[i].isFriend && !enemies[j].isFriend) {
            isAttacking = true;
            if (enemies[i].attackDelayCounter >= enemies[i].attackDelay && !hasAttacked) {
              enemies[j].hp -= enemies[i].damage;
              if (enemies[j].hp <= 0) {
                kills += 1;
              }
              playRawSFX3D(23, enemies[i].x, enemies[i].y); // Play hit sound
              hasAttacked = true;
            }
          }
        }
      }
    }
    
    // Handle attack delay counter
    if (hasAttacked) {
      enemies[i].attackDelayCounter = 0;
    } else if (isAttacking) {
      if (enemies[i].attackDelayCounter < enemies[i].attackDelay) {
        enemies[i].attackDelayCounter++;
      }
    } else {
      enemies[i].attackDelayCounter = enemies[i].attackDelay; // Reset when not attacking anything
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
      if (dungeonMap[projectileTileY][projectileTileX] == Wall || dungeonMap[projectileTileY][projectileTileX] == Bars || dungeonMap[projectileTileY][projectileTileX] == DoorClosed || projectiles[i].x < 0 || projectiles[i].y < 0 || projectiles[i].x > SCREEN_WIDTH || projectiles[i].y > SCREEN_HEIGHT || projectiles[i].speed <= 0 || (projectiles[i].dx == 0 && projectiles[i].dy == 0)) {
        projectiles[i].active = false; // Deactivate the bullet
        playRawSFX3D(22, projectiles[i].x, projectiles[i].y);
      }

      // Check for collisions with enemies
      for (int j = 0; j < maxEnemies; j++) {
        // Skip collision check if this is the enemy that shot the projectile
        if (!projectiles[i].shotByPlayer && j == projectiles[i].shooterId) {
          continue;
        }

        bool collision = false;
        if (enemies[j].name == "boss") {
          // For boss, check collision with full 16x16 sprite using 4 points
          collision = checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y, enemies[j].x, enemies[j].y) ||
                     checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y, enemies[j].x + 1, enemies[j].y) ||
                     checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y, enemies[j].x, enemies[j].y + 1) ||
                     checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y, enemies[j].x + 1, enemies[j].y + 1);
        } else {
          // For regular enemies, use normal 8x8 collision
          if (!enemies[j].isFriend) {// Friendly enemies cannot be hurt
            collision = checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y, enemies[j].x, enemies[j].y);
          }
        }
        
        if (collision && enemies[j].hp > 0) {
          // Hit by player's projectile or another enemy's projectile
          enemies[j].hp -= projectiles[i].damage;    // Reduce enemy health
          playRawSFX3D(23, enemies[j].x, enemies[j].y);
          if (enemies[j].hp <= 0 && projectiles[i].active == true) {
            kills += 1;
            if (enemies[j].name == "clock") {
              enemies[j].x = -3000;
              enemies[j].y = -3000;
              playerNearClockEnemy = false;
            }
          }
          projectiles[i].active = false; // Deactivate the bullet
          break;
        } else if (!damsel[0].dead && !damsel[0].beingCarried && dungeon != bossfightLevel && checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y, damsel[0].x, damsel[0].y)) {
          if (damsel[0].levelOfLove < 6 || !projectiles[i].shotByPlayer) {
            if (!projectiles[i].shotByPlayer) {
              damselDeathMsg = "Something killed ";
            }
            playRawSFX3D(23, damsel[0].x, damsel[0].y);
            playRawSFX3D(17, damsel[0].x, damsel[0].y);
            levelOfDamselDeath = dungeon;
            damsel[0].dead = true;
            currentDamselPortrait = damselPortraitDying;
            dialogueTimeLength = 200;
            currentDialogue = "Ugh-!";
            showDialogue = true;
            damsel[0].active = false;
            projectiles[i].active = false;
            break;
          }
        }
      }

      // Also check for collision with player:
      if (projectiles[i].shotByPlayer == false && checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y, playerX, playerY)) {
        int damage = projectiles[i].damage - round(equippedArmorValue);
        reduceArmorDurability(i);
        if (damage < 0) damage = 0;  // Ensure damage doesn't go below 0
        playerHP -= damage;
        triggerScreenShake(2, 1);
        playRawSFX(0);
        checkIfDeadFrom("projectile");
        projectiles[i].active = false; // Deactivate the bullet
      }
    }
  }
}

void moveDamselToPos(float posX, float posY) {
  damsel[0].x = posX;
  damsel[0].y = posY;
}

void reduceArmorDurability(int i) {
  equippedArmor.armorValue -= ((float)enemies[i].damage / 100);
  if (equippedArmor.armorValue < 0) {
    equippedArmor.armorValue = 0;
    equippedArmor.description = "Broken armor.";
  }
  
  equippedArmorValue = equippedArmor.armorValue;

  //Serial.print("Armor durability reduced to: ");
  //Serial.println(equippedArmor.armorValue);
  //Serial.print("EquippedArmorvalue is now: ");
  //Serial.println(equippedArmorValue);
}

void shootProjectile(float x, float y, float xDir, float yDir, bool shotByPlayer, int shooterId) {

  for (int i = 0; i < maxProjectiles; i++) {
    if (!projectiles[i].active) {
      projectiles[i].x = x;
      projectiles[i].y = y;
      projectiles[i].dx = xDir;  // Set direction based on player's facing direction
      projectiles[i].dy = yDir;
      projectiles[i].damage = shotByPlayer ? playerAttackDamage : 4; // Player projectiles do playerAttackDamage, enemies do 4 damage
      projectiles[i].speed = shotByPlayer? 0.5 : 0.25;
      projectiles[i].active = true;
      projectiles[i].shotByPlayer = shotByPlayer;
      projectiles[i].shooterId = shooterId; // Track which enemy shot this projectile
      break;
    }
  }
}

void renderEnemies() {
  int playerTileX = predictXtile(playerX);
  int playerTileY = predictYtile(playerY);

  for (int i = 0; i < maxEnemies; i++) {
    // Always show boss even if dead, hide other enemies when dead
    if (enemies[i].name == "boss" || enemies[i].hp > 0) {
      int enemyTileX = predictXtile(enemies[i].x);
      int enemyTileY = predictYtile(enemies[i].y);

      // Distance check (10 tiles max)
      int dx = enemyTileX - playerTileX;
      int dy = enemyTileY - playerTileY;
      int distSq = dx*dx + dy*dy;
      if (distSq > 200) continue;

      // Line-of-sight check
      if (seeAll || isVisible(playerTileX, playerTileY, enemyTileX, enemyTileY)) {
        float screenX = (enemies[i].x - offsetX) * tileSize;
        float screenY = (enemies[i].y - offsetY) * tileSize;
        if (screenX >= 0 && screenY >= 0 && screenX < SCREEN_WIDTH && screenY < SCREEN_HEIGHT) {
          Enemy& e = enemies[i];
          if (e.name == "succubus") {
            display.drawBitmap(screenX, screenY, playerX > e.x ? e.sprite : playerX < e.x ? succubusIdleSpriteFlipped : e.sprite, 8, 8, 15);
          } else {
            display.drawBitmap(screenX, screenY, e.sprite, e.name == "boss" ? 16 : 8, e.name == "boss" ? 16 : 8, 15);
          }
        }
      }
    }
  }
}

void renderDamsel() {
  if (damsel[0].beingCarried) return;
  
  // Don't render if damsel is deactivated (succubus is friend)
  if (!damsel[0].active) return;

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