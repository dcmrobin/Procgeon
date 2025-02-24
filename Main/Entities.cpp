#include "Entities.h"
#include "Sprites.h"
#include "HelperFunctions.h"
#include "Dungeon.h"
#include "Player.h"

Damsel damsel[1];
Enemy enemies[maxEnemies];
Projectile projectiles[maxProjectiles];
int levelOfDamselDeath = -4;

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

    // Calculate distance to player
    int dx = round(playerX) - round(enemies[i].x);
    int dy = round(playerY) - round(enemies[i].y);
    int distanceSquared = dx * dx + dy * dy;

    // Check if enemy should chase the player
    if (distanceSquared <= 25) { // Chase if within 5 tiles (distance^2 = 25)
      enemies[i].chasingPlayer = true;
    } else {
      enemies[i].chasingPlayer = false;
    }

    if (enemies[i].chasingPlayer) {
      // Determine the primary direction of movement
      float moveX = (dx > 0 ? 1 : dx < 0 ? -1 : 0);
      float moveY = (dy > 0 ? 1 : dy < 0 ? -1 : 0);

      // Normalize movement vector
      float magnitude = sqrt(moveX * moveX + moveY * moveY);
      if (magnitude > 0) {
        moveX /= magnitude;
        moveY /= magnitude;
      }

      // Attempt to move diagonally first
      float nx = enemies[i].x + moveX * enemies[i].moveAmount;
      float ny = enemies[i].y + moveY * enemies[i].moveAmount;

      bool xValid = !checkSpriteCollisionWithTileX(nx, enemies[i].x, enemies[i].y);
      bool yValid = !checkSpriteCollisionWithTileY(ny, enemies[i].y, enemies[i].x);

      if (xValid && yValid) {
        // Move diagonally if both directions are valid
        enemies[i].x = nx;
        enemies[i].y = ny;
      } else if (xValid) {
        // Slide along X if Y is blocked
        enemies[i].x = nx;
      } else if (yValid) {
        // Slide along Y if X is blocked
        enemies[i].y = ny;
      } else {
        // Both directions blocked, try "wall sliding"
        // Check perpendicular sliding directions
        float slideX = enemies[i].x + moveX * enemies[i].moveAmount;
        float slideY = enemies[i].y;

        if (!checkSpriteCollisionWithTileX(slideX, enemies[i].x, enemies[i].y)) {
          enemies[i].x = slideX;
        } else if (!checkSpriteCollisionWithTileY(slideY, enemies[i].y, enemies[i].x)) {
          enemies[i].y = slideY;
        }
      }
    } else {
      // Random wandering if not chasing
      int dir = random(0, 4);
      float nx = enemies[i].x + (dir == 0 ? enemies[i].moveAmount * 2 : dir == 1 ? -enemies[i].moveAmount * 2 : 0);
      float ny = enemies[i].y + (dir == 2 ? enemies[i].moveAmount * 2 : dir == 3 ? -enemies[i].moveAmount * 2 : 0);

      if (!checkSpriteCollisionWithTileX(nx, enemies[i].x, enemies[i].y)) {
        enemies[i].x = nx;
      }
      if (!checkSpriteCollisionWithTileY(ny, enemies[i].y, enemies[i].x)) {
        enemies[i].y = ny;
      }
    }

    // Check for collision with the player
    if (checkSpriteCollisionWithSprite(playerX, playerY, enemies[i].x, enemies[i].y)) {
      if (atkDelayCounter >= enemies[i].attackDelay) {
        playerHP -= 5; // Damage player
        atkDelayCounter = 0;
      }
      if (playerHP <= 0) {
        deathCause = enemies[i].name;
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
      if (dungeonMap[projectileTileY][projectileTileX] != Floor || projectiles[i].x < 0 || projectiles[i].y < 0 || projectiles[i].x > SCREEN_WIDTH || projectiles[i].y > SCREEN_HEIGHT) {
          projectiles[i].active = false; // Deactivate the bullet
          //free(projectiles[i]);
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
          display.drawBitmap(screenX, screenY, blobSprite, 8, 8, 15);
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