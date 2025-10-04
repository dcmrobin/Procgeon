#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include <Arduino.h>
#include <Adafruit_SSD1327.h>
#include <Adafruit_GFX.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "Dungeon.h"
#include "Entities.h"
#include "Sprites.h"

#define OLED_MOSI 11
#define OLED_CLK 13
#define OLED_DC 14
#define OLED_CS 10
#define OLED_RST 9
#define BUTTON_START_PIN 6
#define BUTTON_UP_PIN    2
#define BUTTON_DOWN_PIN  3
#define BUTTON_LEFT_PIN  4
#define BUTTON_RIGHT_PIN 5
#define BUTTON_B_PIN 1
#define BUTTON_A_PIN 0
#define seedPin A0
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

extern Adafruit_SSD1327 display;
extern U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

struct ButtonStates {
  bool upPressed;
  bool upPressedPrev;
  bool downPressed;
  bool downPressedPrev;
  bool aPressed;
  bool aPressedPrev;
  bool bPressed;
  bool bPressedPrev;
  bool leftPressed;
  bool leftPressedPrev;
  bool rightPressed;
  bool rightPressedPrev;
  bool startPressed;
  bool startPressedPrev;
};

extern ButtonStates buttons;

enum UIState {
  UI_NORMAL,      // Normal gameplay
  UI_INVENTORY,   // Inventory screen
  UI_MINIMAP,     // Minimap screen
  UI_ITEM_ACTION, // Item action selection screen
  UI_ITEM_INFO,   // Item info screen
  UI_ITEM_RESULT, // Item result screen
  UI_PAUSE,       // Pause screen
  UI_RIDDLE       // Riddle screen
};

struct GeneratedRiddle {
  String riddle;       // The riddle text (with inserted attributes)
  String options[4];   // Four answer options (one is correct)
  int correctOption;   // Index (0-3) of the correct answer in options[]
};

struct RiddleAnswer {
  const char* word;
  const char* attributes[4];  // 4 attributes per answer for variety
};

extern GeneratedRiddle currentRiddle;
extern int selectedRiddleOption; 
extern bool riddleGenerated; 

// Add action selection tracking
extern int selectedActionIndex; // 0 = Use, 1 = Drop, 2 = Info

extern UIState currentUIState; // Current UI state

extern bool statusScreen;
extern bool finalStatusScreen
extern bool showDeathScreen;
extern bool credits;

extern const int viewportWidth;
extern const int viewportHeight;

extern float offsetX;
extern float offsetY;

extern int shakeDuration;
extern int shakeIntensity;

extern const float scrollSpeed;

void generateRiddleUI();
void trainFemaleMarkov();
String generateFemaleName();
uint32_t generateRandomSeed();
void carveHorizontalCorridor(int x1, int x2, int y);
void carveVerticalCorridor(int y1, int y2, int x);
void getEdgeTowards(const Room& from, const Room& to, int& outX, int& outY);
void swap(int &a, int &b);
int countWalls(int x, int y);
int predictXtile(float x);
int predictYtile(float y);
bool checkSpriteCollisionWithTileX(float newX, float currentX, float newY);
bool checkSpriteCollisionWithTileY(float newY, float currentY, float newX);
bool checkSpriteCollisionWithSprite(float sprite1X, float sprite1Y, float sprite2X, float sprite2Y);
void updateButtonStates();
void handleUIStateTransitions();
void updateAnimations();
void renderUI();
bool isVisible(int x0, int y0, int x1, int y1);
bool isWalkable(int x, int y);
void drawWrappedText(int x, int y, int maxWidth, const String &text);
void updateScreenShake();
void triggerScreenShake(int duration, int intensity);
bool nearTile(TileTypes tile);
void checkIfDeadFrom(const String &cause);

#endif