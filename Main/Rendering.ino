// Draw a tile based on its type
void drawTile(int mapX, int mapY, int screenX, int screenY) {
  int tileType = dungeonMap[mapY][mapX];

  switch (tileType) {
    case 0: // Start stairs
      u8g2.drawXBMP(screenX, screenY, tileSize, tileSize, stairsSprite);
      break;
    case 1: // Floor
      //u8g2.drawFrame(screenX, screenY, tileSize, tileSize);
      break;
    case 2: // Wall
      //u8g2.drawBox(screenX, screenY, tileSize, tileSize);
      u8g2.drawXBMP(screenX, screenY, tileSize, tileSize, wallSprite);
      break;
    case 4: // Exit
      u8g2.drawXBMP(screenX, screenY, tileSize, tileSize, stairsSprite);
      break;
  }
}

// Render the visible portion of the dungeon
void renderDungeon() {
  for (int y = 0; y < viewportHeight; y++) {
    for (int x = 0; x < viewportWidth; x++) {
      int mapX = x + offsetX;
      int mapY = y + offsetY;

      if (mapX >= 0 && mapX < mapWidth && mapY >= 0 && mapY < mapHeight) {
        drawTile(mapX, mapY, x * tileSize, y * tileSize);
      }
    }
  }
}

// Render the player
void renderPlayer() {
  int screenX = (playerX - offsetX) * tileSize;
  int screenY = (playerY - offsetY) * tileSize;

  // Ensure the player is within the viewport
  if (screenX >= 0 && screenX < 128 && screenY >= 0 && screenY < 128) {
    //u8g2.drawDisc(screenX + tileSize / 2, screenY + tileSize / 2, tileSize / 3, U8G2_DRAW_ALL);
    u8g2.drawXBMP((screenX + tileSize / 2) - tileSize/2, (screenY + tileSize / 2) - tileSize/2, tileSize, tileSize, playerSprite);
  }
}

void renderEnemies() {
  for (int i = 0; i < maxEnemies; i++) {
    if (enemies[i].hp > 0) {
      int screenX = (enemies[i].x - offsetX) * tileSize;
      int screenY = (enemies[i].y - offsetY) * tileSize;
      if (screenX >= 0 && screenY >= 0 && screenX < 128 && screenY < 128) {
        u8g2.drawXBMP(screenX, screenY, 8, 8, blobSprite);
      }
    }
  }
}

void renderProjectiles() {
    for (int i = 0; i < maxProjectiles; i++) {
        if (projectiles[i].active) {
          int screenX = (projectiles[i].x - offsetX) * tileSize + tileSize/2;
          int screenY = (projectiles[i].y - offsetY) * tileSize + tileSize/2;
          u8g2.drawDisc(screenX, screenY, 1);
        }
    }
}

// Render the UI
void renderUI() { 
  char HP[4];
  char Lvl[7];
  snprintf(HP, sizeof(HP), "%d", playerHP); // Convert playerHP to a string
  snprintf(Lvl, sizeof(Lvl), "%d", level);
  
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(5, 123, "HP:");
  u8g2.drawStr(20, 123, HP);
  u8g2.drawStr(40, 123, "LVL:");
  u8g2.drawStr(60, 123, Lvl);
  u8g2.drawFrame(0, 113, 128, 15);
}