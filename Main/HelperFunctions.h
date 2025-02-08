#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <Arduino.h>
#include <U8g2lib.h>
#include "Dungeon.h"
#include "Entities.h"
#include "Sprites.h"

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

extern U8G2_SH1107_PIMORONI_128X128_F_4W_HW_SPI u8g2;

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
  UI_ITEM_INFO,   // Item info screen
  UI_ITEM_RESULT  // Item result screen
};

// Add action selection tracking
extern int selectedActionIndex; // 0 = Use, 1 = Drop, 2 = Info

extern UIState currentUIState; // Current UI state

extern bool statusScreen;

extern const int viewportWidth;
extern const int viewportHeight;

extern float offsetX;
extern float offsetY;

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
void handleUIStateTransitions(bool hasMap);
void updateAnimations();
void drawWrappedText(const char *text, int x, int y, int maxWidth, int lineHeight);

#endif