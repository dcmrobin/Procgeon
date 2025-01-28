#include "Entities.h"
#include "Sprites.h"
#include "HelperFunctions.h"

Damsel damsel[1];
Enemy enemies[maxEnemies];

int atkDelayCounter = 0;
void updateEnemies(int& playerHP, float playerX, float playerY, const char*& deathCause) {
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
    if (checkSpriteCollisionWithSprite(enemies[i].x, enemies[i].y, playerX, playerY)) {
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