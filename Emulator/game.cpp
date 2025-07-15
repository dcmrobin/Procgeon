#include "game.h"
#include "Adafruit_SSD1327_emu.h"

extern Adafruit_SSD1327 display; // Defined in main.cpp

static int playerX = 64;
static int playerY = 64;

void game_setup() {
    display.clearDisplay();
    display.fillRect(0, 0, 128, 128, 0);
}

// Draw a bouncing ball
void game_loop() {
    static int x = 64, y = 64;
    static int dx = 2, dy = 3;
    
    // Clear previous frame
    display.fillScreen(0);
    
    // Update position
    x += dx;
    y += dy;
    if (x <= 5 || x >= 123) dx = -dx;
    if (y <= 5 || y >= 123) dy = -dy;
    
    // Draw ball
    display.fillCircle(x, y, 5, 15);
    
    // Draw score
    display.setTextSize(1);
    display.setTextColor(15);
    display.setCursor(5, 5);
    display.print("Score: 42");
    
    // Draw obstacles
    display.drawRoundRect(20, 20, 30, 10, 3, 7);
    display.fillTriangle(80, 100, 90, 90, 100, 100, 7);
}