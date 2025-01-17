int counter = 0;
void updateAnimations() {
  counter += 1;
  if (counter >= 20) {
    blobSprite = blobSprite == blobSpriteFrame1 ? blobSpriteFrame2 : blobSpriteFrame1;
    counter = 0;
  }
}

void updateScrolling() {
  // Target offsets based on player's position
  float targetOffsetX = playerX - (viewportWidth / 2.0f) + 0.5f;
  float targetOffsetY = playerY - (viewportHeight / 2.0f) + 0.5f;

  // Clamp target offsets to map boundaries
  targetOffsetX = constrain(targetOffsetX, 0, mapWidth - viewportWidth);
  targetOffsetY = constrain(targetOffsetY, 0, mapHeight - viewportHeight);

  // Smoothly move the offset towards the target
  offsetX += (targetOffsetX - offsetX) * scrollSpeed;
  offsetY += (targetOffsetY - offsetY) * scrollSpeed;
}

int atkDelayCounter = 0;
void updateEnemies() {
  atkDelayCounter += 1;
  for (int i = 0; i < maxEnemies; i++) {
    if (enemies[i].hp <= 0) continue; // Skip dead enemies

    // Calculate distance to player
    int dx = playerX - (int)enemies[i].x;
    int dy = playerY - (int)enemies[i].y;
    int distanceSquared = dx * dx + dy * dy;

    // Check if enemy should chase the player
    if (distanceSquared <= 25) { // Chase if within 5 tiles (distance^2 = 25)
      enemies[i].chasingPlayer = true;
    } else {
      enemies[i].chasingPlayer = false;
    }

    if (enemies[i].chasingPlayer) {
      // Move diagonally or straight toward the player
      float moveX = (dx > 0 ? 1 : dx < 0 ? -1 : 0);
      float moveY = (dy > 0 ? 1 : dy < 0 ? -1 : 0);

      // Normalize movement vector to prevent faster diagonal movement
      float magnitude = sqrt(moveX * moveX + moveY * moveY);
      if (magnitude > 0) {
        moveX /= magnitude;
        moveY /= magnitude;
      }

      // Calculate potential new position
      float nx = enemies[i].x + moveX * enemies[i].moveAmount;
      float ny = enemies[i].y + moveY * enemies[i].moveAmount;

      int ptx = predictXtile(nx);
      int pty = predictYtile(ny);

      int ctx = round(nx);
      int cty = round(ny);

      // Check if the new position collides with walls
      if (dungeonMap[cty][ptx] != 1) {
        nx = enemies[i].x;
      }
      if (dungeonMap[pty][ctx] != 1) {
        ny = enemies[i].y;
      }

      // Check bounds and ensure the move is valid
      if (nx >= 0 && ny >= 0 && nx < mapWidth && ny < mapHeight && dungeonMap[pty][ptx] == 1) {
        enemies[i].x = nx;
        enemies[i].y = ny;
      }
    } else {
      // Random wandering
      int dir = random(0, 4);
      float nx = enemies[i].x + (dir == 0 ? enemies[i].moveAmount*2 : dir == 1 ? -(enemies[i].moveAmount)*2 : 0);
      float ny = enemies[i].y + (dir == 2 ? enemies[i].moveAmount*2 : dir == 3 ? -(enemies[i].moveAmount)*2 : 0);

      int ptx = predictXtile(nx);
      int pty = predictYtile(ny);

      int ctx = round(nx);
      int cty = round(ny);

      if (dungeonMap[cty][ptx] != 1) {
        nx = enemies[i].x;
      }
      if (dungeonMap[pty][ctx] != 1) {
        ny = enemies[i].y;
      }

      // Check bounds and avoid walls
      if (nx >= 0 && ny >= 0 && nx < mapWidth && ny < mapHeight && dungeonMap[pty][ptx] == 1) {
        enemies[i].x = nx;
        enemies[i].y = ny;
      }
    }

    int enemyRoundX = round(enemies[i].x);
    int enemyRoundY = round(enemies[i].y);
    int playerRoundX = round(playerX);
    int playerRoundY = round(playerY);

    // Check for collision with the player
    if (enemyRoundX == playerRoundX && enemyRoundY == playerRoundY) {
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
          if (dungeonMap[projectileTileY][projectileTileX] != 1 || projectiles[i].x < 0 || projectiles[i].y < 0 || projectiles[i].x > 128 || projectiles[i].y > 128) {
              projectiles[i].active = false; // Deactivate the bullet
              //free(projectiles[i]);
          }

          // Check for collisions with enemies
          for (int j = 0; j < maxEnemies; j++) {
            int enemyRoundX = round(enemies[j].x);
            int enemyRoundY = round(enemies[j].y);
            int projectileRoundX = round(projectiles[i].x);
            int projectileRoundY = round(projectiles[i].y);

            if (projectileRoundX == enemyRoundX && projectileRoundY == enemyRoundY && enemies[j].hp > 0) {
              enemies[j].hp -= projectiles[i].damage;    // Reduce enemy health
              if (enemies[j].hp <= 0 && projectiles[i].active == true) {
                kills += 1;
              }
              projectiles[i].active = false; // Deactivate the bullet
            }
          }
      }
  }
}