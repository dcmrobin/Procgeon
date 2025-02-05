#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <Arduino.h>

#define OLED_MOSI 11
#define OLED_CLK 13
#define OLED_DC 7
#define OLED_CS 10
#define OLED_RST 9
#define BUTTON_UP_PIN    2
#define BUTTON_DOWN_PIN  3
#define BUTTON_LEFT_PIN  4
#define BUTTON_RIGHT_PIN 5
#define BUTTON_B_PIN 1
#define BUTTON_A_PIN 0
#define seedPin A0
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

struct ButtonStates {
  bool upPressed;
  bool upPressedPrev;
  bool downPressed;
  bool downPressedPrev;
  bool aPressed;
  bool aPressedPrev;
  bool bPressed;
  bool bPressedPrev;
};

extern ButtonStates buttons;

enum UIState {
  UI_NORMAL,      // Normal gameplay
  UI_INVENTORY,   // Inventory screen
  UI_MINIMAP,     // Minimap screen
  UI_ITEM_ACTION, // Item action selection screen
  UI_ITEM_INFO    // Item info screen
};

// Add action selection tracking
extern int selectedActionIndex; // 0 = Use, 1 = Drop, 2 = Info

extern UIState currentUIState; // Current UI state

uint32_t generateRandomSeed();
void carveHorizontalCorridor(int x1, int x2, int y);
void carveVerticalCorridor(int y1, int y2, int x);
void swap(int &a, int &b);
int countWalls(int x, int y);
int predictXtile(float x);
int predictYtile(float y);
bool checkSpriteCollisionWithTileX(float newX, float currentX, float newY);
bool checkSpriteCollisionWithTileY(float newY, float currentY, float newX);
bool checkSpriteCollisionWithSprite(float sprite1X, float sprite1Y, float sprite2X, float sprite2Y);
void updateButtonStates();

#endif