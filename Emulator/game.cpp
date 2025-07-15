#include "game.h"
#include "Adafruit_SSD1327_emu.h"

extern Adafruit_SSD1327 display; // Defined in main.cpp

static int playerX = 64;
static int playerY = 64;

void game_setup() {
    display.clearDisplay();
    display.fillRect(0, 0, 128, 128, SSD1327_BLACK);
}

void game_loop() {
    //display.fillScreen(0);
    
    // Update position (simple movement)
    static int dx = 1, dy = 1;
    playerX += dx;
    playerY += dy;
    
    if (playerX <= 5 || playerX >= 123) dx = -dx;
    if (playerY <= 5 || playerY >= 123) dy = -dy;
    
    // Draw player
    display.fillRect(playerX - 5, playerY - 5, 10, 10, SSD1327_WHITE);
    
    // Draw obstacles
    display.fillRect(20, 20, 30, 10, SSD1327_GRAY);
    display.fillRect(80, 100, 30, 10, SSD1327_GRAY);
}