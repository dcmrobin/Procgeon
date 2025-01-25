#include <U8g2lib.h>
#include <EEPROM.h>

// OLED display pins
#define OLED_MOSI 11
#define OLED_CLK 13
#define OLED_DC 7
#define OLED_CS 10
#define OLED_RST 9
#define seedPin A0
#define BUTTON_UP_PIN    2
#define BUTTON_DOWN_PIN  3
#define BUTTON_LEFT_PIN  4
#define BUTTON_RIGHT_PIN 5
#define BUTTON_B_PIN 1
#define BUTTON_A_PIN 0

const int mapWidth = 64;   // Total map width in tiles
const int mapHeight = 64;  // Total map height in tiles
const int tileSize = 8;    // Size of each tile (in pixels)

// Viewport size (in tiles)
const int viewportWidth = 128 / tileSize;
const int viewportHeight = 128 / tileSize - 2;

// Map scrolling offset
float offsetX = 0;
float offsetY = 0;

// Smooth scrolling speed
const float scrollSpeed = 0.25f;

// Player position
float playerX = 1;
float playerY = 1;

// Player stats
int playerHP = 100;
int level = 1;
int kills = 0;
unsigned int lvlHighscoreAddress = 0;
unsigned int killHighscoreAddress = 1;
const char* deathCause = "";

// Dungeon map (2D array)
int dungeonMap[mapHeight][mapWidth];

static const unsigned char PROGMEM stairsSprite[] =
{ 0b11000000,
  0b11000000,
  0b11110000,
  0b11110000,
  0b11111100,//sprite is flipped on its x axis for some reason
  0b11111100,
  0b11111111,
  0b11111111
};

static const unsigned char PROGMEM wallSprite[] =
{ 
  0b00000000, 
  0b11101111, 
  0b00000000, 
  0b10111011, 
  0b10111011, 
  0b00000000, 
  0b11101110, 
  0b11101110
};

static const unsigned char PROGMEM barsSprite[] =
{ 
  0b11111111, 
  0b01110111, 
  0b00100010, 
  0b00100010, 
  0b00100010, 
  0b00100010, 
  0b01110111, 
  0b11111111
};

static const unsigned char PROGMEM playerSpriteRight[] =
{ 
  0b00001100, 
  0b00111000, 
  0b00011100, 
  0b01011000, 
  0b01111100, 
  0b01011100, 
  0b01011100, 
  0b01011110
};

static const unsigned char PROGMEM playerSpriteLeft[] =
{ 
  0b00110000, 
  0b00011100, 
  0b00111000, 
  0b00011010, 
  0b00111110, 
  0b00111010, 
  0b00111010, 
  0b01111010
};

const unsigned char* playerSprite = playerSpriteLeft;

static const unsigned char PROGMEM damselSpriteRight[] =
{ 
  0b00000000, 
  0b00011000, 
  0b00111100, 
  0b00011110, 
  0b00111100, 
  0b01011010, 
  0b00111100, 
  0b01111110
};

static const unsigned char PROGMEM damselSpriteLeft[] =
{ 
  0b00000000, 
  0b00011000, 
  0b00111100, 
  0b01111000, 
  0b00111100, 
  0b01011010, 
  0b00111100, 
  0b01111110
};

static const unsigned char PROGMEM damselHopefullSpriteRight[] =
{ 
  0b00000000, 
  0b00011000, 
  0b00111100, 
  0b00011110, 
  0b01111100, 
  0b00011010, 
  0b00111100, 
  0b01111110
};

static const unsigned char PROGMEM damselHopefullSpriteLeft[] =
{ 
  0b00000000, 
  0b00011000, 
  0b00111100, 
  0b01111000, 
  0b00111110, 
  0b01011000, 
  0b00111100, 
  0b01111110
};

static const unsigned char PROGMEM damselSpriteDead[] =
{ 
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b00001100, 
  0b11011110, 
  0b11111111
};

const unsigned char* damselSprite = damselSpriteLeft;

static const unsigned char PROGMEM blobSpriteFrame1[] =
{ 
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b00111100, 
  0b01011110, 
  0b01111110, 
  0b01111110, 
  0b01111110
};

static const unsigned char PROGMEM blobSpriteFrame2[] =
{ 
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b00111100, 
  0b01011110, 
  0b01111110, 
  0b11111111
};

const unsigned char* blobSprite = blobSpriteFrame1;

const unsigned char rescueDamselScreen [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0xa0, 0x8a, 0xaa, 0x2a, 0xaa, 0xaa, 0xa8, 0xaa, 0xa2, 0xaa, 0x8a, 0xaa, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x51, 0x55, 0x45, 0x55, 
	0x00, 0x00, 0x00, 0x00, 0xaa, 0x8a, 0xaa, 0x2a, 0xaa, 0xaa, 0xa8, 0xaa, 0xa2, 0xaa, 0x8a, 0xaa, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x54, 0x55, 0x51, 0x55, 0x45, 0x55, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x20, 0x2a, 0xaa, 0xa8, 0xa8, 0xa2, 0xa2, 0x8a, 0x8a, 0x2a, 0x2a, 0xaa, 0xa8, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x45, 0x45, 0x15, 0x15, 0x55, 0x54, 0x54, 0x51, 
	0x00, 0x00, 0x00, 0x2a, 0x2a, 0xaa, 0xa8, 0xa8, 0xa2, 0xa2, 0x8a, 0x8a, 0x2a, 0x2a, 0xaa, 0xa8, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x45, 0x45, 0x15, 0x15, 0x55, 0x54, 0x54, 0x51, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x80, 0xa2, 0xaa, 0x8a, 0xaa, 0x2a, 0xaa, 0xaa, 0xa8, 0xaa, 0xa2, 0xaa, 0x8a, 0xaa, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x55, 0x55, 0x54, 0x55, 0x51, 0x55, 0x45, 0x55, 
	0x00, 0x00, 0xa8, 0xa2, 0xaa, 0x8a, 0xaa, 0x2a, 0xaa, 0xaa, 0xa8, 0xaa, 0xa2, 0xaa, 0x8a, 0xaa, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x15, 0x55, 0x55, 0x54, 0x55, 0x51, 0x55, 0x45, 0x55, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xaa, 0xa2, 0xaa, 0x8a, 0xaa, 0x2a, 0xaa, 0xaa, 0xa8, 0xaa, 0xa2, 0xaa, 0x8a, 0xaa, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x15, 0x55, 0x55, 0x54, 0x55, 0x51, 0x55, 0x45, 0x55, 
	0x00, 0x80, 0xaa, 0xa2, 0xaa, 0x8a, 0xaa, 0x2a, 0xaa, 0xaa, 0xa8, 0xaa, 0xa2, 0xaa, 0x8a, 0xaa, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x55, 0x15, 0x55, 0x55, 0x54, 0x55, 0x71, 0x7e, 0x46, 0x55, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x80, 0x8a, 0x2a, 0x2a, 0xaa, 0xa8, 0xa8, 0xa2, 0xa2, 0x8a, 0xca, 0xf3, 0xff, 0xcf, 0xa9, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x51, 0x51, 0x45, 0x45, 0x15, 0xf5, 0xf3, 0xff, 0xcf, 0x53, 
	0x00, 0x88, 0x8a, 0x2a, 0x2a, 0xaa, 0xa8, 0xa8, 0xa2, 0xa2, 0x8a, 0xfa, 0xf3, 0xff, 0xcf, 0xaf, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x51, 0x51, 0x45, 0x45, 0x15, 0xfd, 0xf3, 0xff, 0xcf, 0x7f, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xa8, 0xaa, 0xa2, 0xaa, 0x8a, 0xaa, 0x2a, 0xaa, 0xaa, 0xa8, 0x9f, 0x7f, 0x7e, 0xfe, 0xf9, 
	0x00, 0x00, 0x00, 0x00, 0x40, 0x45, 0x55, 0x15, 0x55, 0x55, 0x94, 0x9f, 0x7f, 0x7e, 0xfe, 0xf9, 
	0x00, 0xa8, 0xaa, 0xa2, 0xaa, 0x8a, 0xaa, 0x2a, 0xaa, 0xaa, 0x98, 0x9f, 0x7f, 0x7e, 0xfe, 0xf9, 
	0x00, 0x00, 0x00, 0x00, 0x40, 0x45, 0x55, 0x15, 0x55, 0x55, 0x94, 0x9f, 0x7f, 0x7e, 0xfe, 0xf9, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x80, 0xa8, 0xaa, 0xa2, 0xaa, 0x8a, 0xaa, 0x2a, 0xaa, 0xaa, 0xfc, 0xff, 0xf3, 0xff, 0xcf, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x54, 0x45, 0x55, 0x15, 0x55, 0x55, 0xfc, 0xff, 0xf3, 0xff, 0xcf, 0xff, 
	0x80, 0xa8, 0xaa, 0xa2, 0xaa, 0x8a, 0xaa, 0x2a, 0xaa, 0xaa, 0xfc, 0xff, 0xf3, 0xff, 0xcf, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x54, 0x45, 0x55, 0x15, 0x55, 0x55, 0xfc, 0xff, 0xf3, 0xff, 0xcf, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x80, 0x8a, 0x8a, 0x2a, 0x2a, 0xaa, 0xa8, 0xa8, 0xa2, 0xa2, 0x9f, 0x9f, 0x7f, 0x7e, 0xfe, 0xf9, 
	0x00, 0x00, 0x00, 0x00, 0x54, 0x54, 0x51, 0x51, 0x45, 0x05, 0x98, 0x9f, 0x7f, 0x7e, 0xfe, 0xf9, 
	0x80, 0x8a, 0x8a, 0x2a, 0x2a, 0xaa, 0xa8, 0xa8, 0xa2, 0x82, 0x93, 0x9f, 0x7f, 0x7e, 0xfe, 0xf9, 
	0x00, 0x00, 0x00, 0x00, 0x54, 0x54, 0x51, 0x51, 0x45, 0xe5, 0x97, 0x9f, 0x7f, 0x7e, 0xfe, 0xf9, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xa0, 0xa8, 0xaa, 0xa2, 0xaa, 0x8a, 0xaa, 0x2a, 0xaa, 0xaa, 0xce, 0xff, 0xf3, 0xff, 0xcf, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x54, 0x45, 0x55, 0x15, 0x55, 0x35, 0xdf, 0xff, 0xf3, 0xff, 0xcf, 0xff, 
	0xa0, 0xa8, 0xaa, 0xa2, 0xaa, 0x8a, 0xaa, 0x2a, 0xaa, 0xba, 0x9e, 0xff, 0xf3, 0xff, 0xcf, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x54, 0x45, 0x55, 0x15, 0x55, 0x35, 0xbf, 0xff, 0xf3, 0xff, 0xcf, 0xff, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xa0, 0xa8, 0xaa, 0x22, 0xa8, 0x83, 0xaa, 0x2a, 0xaa, 0x66, 0x7e, 0x9f, 0x7f, 0xa0, 0xf0, 0xf9, 
	0x00, 0x00, 0x00, 0x00, 0xfc, 0x47, 0x55, 0x15, 0x55, 0x65, 0x7d, 0x9e, 0x7f, 0x54, 0xe5, 0xf9, 
	0xa0, 0xa8, 0xaa, 0x82, 0xfa, 0x87, 0xaa, 0x2a, 0xaa, 0x66, 0xfe, 0x80, 0x7f, 0x0a, 0xea, 0xf9, 
	0x00, 0x00, 0x00, 0x00, 0xfd, 0x47, 0x55, 0x15, 0x55, 0x25, 0xff, 0x8f, 0x7f, 0xe4, 0xe4, 0xf9, 
	0x00, 0x00, 0x00, 0x80, 0xfe, 0x0f, 0x00, 0x00, 0x00, 0x80, 0xfe, 0x07, 0x00, 0xb2, 0x09, 0x00, 
	0x00, 0x00, 0x00, 0x40, 0xfd, 0x0f, 0x00, 0x00, 0x00, 0x54, 0x3f, 0x00, 0x00, 0x51, 0x11, 0x00, 
	0xa0, 0x8a, 0x8a, 0x92, 0xff, 0xaf, 0xa8, 0xa8, 0xa2, 0xf8, 0xc3, 0xfe, 0x73, 0xb2, 0xc9, 0xff, 
	0x00, 0x00, 0x00, 0x48, 0x1f, 0x40, 0x51, 0x51, 0x45, 0x03, 0xfc, 0xfe, 0x73, 0x51, 0xd1, 0xff, 
	0xa0, 0x8a, 0x8a, 0xa0, 0xef, 0xa9, 0xa8, 0xa8, 0xa2, 0x7e, 0xfa, 0xfe, 0x73, 0xb2, 0xc9, 0xff, 
	0x00, 0x00, 0x00, 0xc4, 0xf7, 0x55, 0x51, 0x51, 0x45, 0x7d, 0xfd, 0xfe, 0xf3, 0x54, 0xc5, 0xff, 
	0x00, 0x00, 0x00, 0xa0, 0xf7, 0x01, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x00, 0x00, 0xaa, 0x0a, 0x00, 
	0x00, 0x00, 0x00, 0xd0, 0xef, 0x01, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x54, 0x05, 0x00, 
	0x80, 0xa8, 0xaa, 0xea, 0xef, 0x88, 0xaa, 0x2a, 0xaa, 0x8a, 0xaa, 0x03, 0x7f, 0xa0, 0xf0, 0xf9, 
	0x00, 0x00, 0x00, 0xd0, 0x11, 0x44, 0x55, 0x15, 0x55, 0x41, 0xbd, 0x7f, 0x60, 0xee, 0xfe, 0xf9, 
	0x80, 0xa8, 0x2a, 0xe6, 0xb4, 0x88, 0xaa, 0x2a, 0xaa, 0xa8, 0xac, 0xff, 0x0f, 0xc0, 0xfe, 0xf9, 
	0x00, 0x00, 0x00, 0x08, 0xce, 0x43, 0x55, 0x15, 0x55, 0x56, 0x53, 0xff, 0xff, 0x67, 0xfe, 0xf9, 
	0x00, 0x00, 0x00, 0x00, 0xf7, 0x0d, 0x00, 0x00, 0x00, 0xff, 0x8b, 0xff, 0xff, 0xb7, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x80, 0xef, 0x3b, 0x00, 0x00, 0xc0, 0xff, 0x47, 0xff, 0xff, 0xb7, 0x00, 0x00, 
	0x80, 0xa8, 0xaa, 0xc2, 0xfe, 0xff, 0xa8, 0x2a, 0xf0, 0xff, 0xa7, 0xff, 0xaa, 0x77, 0xce, 0xff, 
	0x00, 0x00, 0x00, 0x60, 0xfd, 0xf7, 0x43, 0x15, 0xfc, 0xf5, 0xcf, 0x0f, 0x55, 0xd5, 0xce, 0xff, 
	0x80, 0xa8, 0xaa, 0xf2, 0xba, 0xab, 0x1f, 0x0a, 0xff, 0xea, 0xa7, 0x67, 0x80, 0xa2, 0xce, 0xff, 
	0x00, 0x00, 0x00, 0x58, 0xdd, 0x51, 0xff, 0x40, 0x7f, 0xd5, 0xd7, 0xf7, 0x33, 0xe4, 0xce, 0xff, 
	0x00, 0x00, 0x00, 0xbc, 0xf8, 0x80, 0xfa, 0xa2, 0xbe, 0xa8, 0xe7, 0x07, 0x00, 0xe0, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x56, 0xf4, 0x00, 0x74, 0x75, 0x5e, 0xd0, 0xd7, 0x03, 0x00, 0xe0, 0x00, 0x00, 
	0x00, 0x8a, 0x0a, 0x2f, 0xaa, 0xaa, 0x60, 0xba, 0x3e, 0xa2, 0xeb, 0x9b, 0x7f, 0xee, 0xfe, 0xf9, 
	0x00, 0x00, 0x80, 0x15, 0x52, 0x54, 0x41, 0x71, 0x1f, 0x55, 0xf3, 0x9b, 0x7f, 0xee, 0xfe, 0xf9, 
	0x00, 0x8a, 0x8a, 0x0a, 0x8b, 0xaa, 0x08, 0x00, 0x8e, 0xa2, 0xeb, 0x99, 0x7f, 0xee, 0xfe, 0xf9, 
	0x00, 0x00, 0x20, 0x85, 0x2e, 0x54, 0x51, 0x51, 0x46, 0x55, 0xf5, 0x9d, 0x7f, 0xce, 0xfe, 0xf9, 
	0x00, 0x00, 0x68, 0x42, 0xbf, 0x00, 0x00, 0x00, 0x00, 0xa0, 0xf9, 0x01, 0x00, 0xe0, 0x00, 0x00, 
	0x00, 0x00, 0x54, 0xa1, 0xff, 0x00, 0x00, 0x00, 0x00, 0x50, 0xf5, 0x01, 0x00, 0xc0, 0x00, 0x00, 
	0x00, 0xa8, 0x28, 0xe0, 0xff, 0x89, 0xaa, 0x2a, 0xaa, 0xa2, 0xfa, 0xfc, 0xf3, 0xaf, 0xce, 0xff, 
	0x00, 0x00, 0x00, 0xd0, 0xff, 0x45, 0x55, 0x15, 0x55, 0x45, 0x00, 0xfe, 0xf3, 0xcf, 0xce, 0xff, 
	0x00, 0xa8, 0xaa, 0xe2, 0xbf, 0x89, 0xaa, 0x2a, 0xaa, 0x0a, 0xec, 0xfe, 0xf3, 0xaf, 0xce, 0xff, 
	0x00, 0x00, 0x00, 0xd0, 0xdf, 0x43, 0x55, 0x15, 0x55, 0xe5, 0x09, 0xfe, 0xf3, 0xcf, 0xce, 0xff, 
	0x00, 0x00, 0x00, 0xe8, 0xbf, 0x03, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0xa0, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0xf0, 0xdf, 0x03, 0x00, 0x00, 0x00, 0x50, 0xfd, 0x01, 0x00, 0xc0, 0x00, 0x00, 
	0x00, 0xa0, 0xaa, 0xe8, 0xbe, 0x8b, 0xaa, 0x2a, 0xaa, 0x62, 0xfe, 0x93, 0x7f, 0xae, 0xfe, 0xf9, 
	0x00, 0x00, 0x00, 0x74, 0x5f, 0x43, 0x55, 0x15, 0x55, 0x11, 0xfd, 0x97, 0x7f, 0xce, 0xfe, 0xf9, 
	0x00, 0x80, 0xaa, 0xe8, 0xbe, 0x8b, 0xaa, 0x2a, 0xaa, 0xaa, 0xfe, 0x87, 0x7f, 0xae, 0xfe, 0xb9, 
	0x00, 0x00, 0x00, 0x74, 0x5f, 0x43, 0x55, 0x15, 0x55, 0x31, 0xff, 0x4f, 0x7f, 0xce, 0xfe, 0x55, 
	0x00, 0x00, 0x00, 0xba, 0xbe, 0x07, 0x00, 0x00, 0x00, 0xa8, 0xfe, 0x0f, 0x00, 0xa0, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x7d, 0x7f, 0x07, 0x00, 0x00, 0x00, 0x50, 0xff, 0x1f, 0x00, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0x08, 0xba, 0xbf, 0xa6, 0xa8, 0xa8, 0xa2, 0x88, 0xff, 0x9f, 0x2a, 0xaa, 0xaa, 0xa8, 
	0x00, 0x00, 0x00, 0x7d, 0x7f, 0x47, 0x51, 0x51, 0x45, 0x55, 0xff, 0x1f, 0x55, 0xc4, 0x54, 0x51, 
	0x00, 0x00, 0x80, 0xbe, 0xbf, 0x86, 0xa8, 0xa8, 0xa2, 0x98, 0xff, 0xbf, 0x2a, 0xaa, 0xaa, 0xa8, 
	0x00, 0x00, 0x40, 0x7f, 0x7f, 0x4f, 0x51, 0x51, 0x45, 0x44, 0xff, 0x3f, 0x55, 0xc4, 0x54, 0x51, 
	0x00, 0x00, 0xa0, 0xbe, 0xff, 0x0e, 0x00, 0x00, 0x00, 0xaa, 0xff, 0x3f, 0x00, 0xa0, 0x00, 0x00, 
	0x00, 0x00, 0x50, 0x7d, 0x7f, 0x0f, 0x00, 0x00, 0x00, 0xc4, 0xff, 0x3f, 0x00, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0xa0, 0xbe, 0xff, 0x0e, 0x00, 0x00, 0x00, 0xaa, 0xff, 0x7f, 0x00, 0xa0, 0x00, 0x00, 
	0x00, 0x00, 0x50, 0x5f, 0x7f, 0x1f, 0x00, 0x00, 0x00, 0xd4, 0xff, 0x7f, 0x00, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0xa8, 0xbe, 0xff, 0x1e, 0x00, 0x00, 0x00, 0xa2, 0xff, 0x7f, 0x00, 0xa0, 0x00, 0x00, 
	0x00, 0x00, 0x54, 0xdf, 0x7f, 0x1d, 0x00, 0x00, 0x00, 0xd5, 0xff, 0xff, 0x00, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0xaa, 0xae, 0xff, 0x3e, 0x00, 0x00, 0x80, 0xa2, 0xff, 0xff, 0x00, 0xa0, 0x00, 0x00, 
	0x00, 0x00, 0x55, 0xdd, 0xff, 0x3d, 0x00, 0x00, 0x00, 0xd1, 0xff, 0xff, 0x00, 0xc0, 0x00, 0x00, 
	0x00, 0x80, 0xaa, 0xee, 0xff, 0x3a, 0x00, 0x00, 0x80, 0xaa, 0xff, 0xff, 0x01, 0xa0, 0x00, 0x00, 
	0x00, 0x40, 0x55, 0xd7, 0xff, 0x7d, 0x00, 0x00, 0x40, 0xd1, 0xff, 0xff, 0x01, 0xc0, 0x00, 0x00, 
	0x00, 0x80, 0xaa, 0xef, 0xff, 0x7b, 0x00, 0x00, 0x80, 0xaa, 0xff, 0xff, 0x03, 0xa0, 0x00, 0x00, 
	0x00, 0x40, 0x55, 0xf7, 0xff, 0x75, 0x00, 0x00, 0x40, 0x51, 0xff, 0xff, 0x03, 0xc0, 0x00, 0x00, 
	0x00, 0xa0, 0xaa, 0xea, 0xff, 0x7b, 0x00, 0x00, 0xa0, 0xa2, 0xaa, 0xea, 0x03, 0xa0, 0x00, 0x00, 
	0x00, 0x50, 0x55, 0xd5, 0xd7, 0x77, 0x00, 0x00, 0x50, 0x15, 0x55, 0xf5, 0x01, 0xc0, 0x00, 0x00, 
	0x00, 0xa8, 0xaa, 0xea, 0xeb, 0xab, 0x00, 0x00, 0xa8, 0x2a, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x00, 
	0x00, 0x54, 0x55, 0x55, 0x55, 0x55, 0x00, 0x00, 0x54, 0x55, 0x55, 0x56, 0x00, 0xc0, 0x00, 0x00, 
	0x00, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0xa8, 0xaa, 0xaa, 0xa8, 0x00, 0xa0, 0x00, 0x00, 
	0x00, 0x55, 0x55, 0x55, 0x55, 0x55, 0x00, 0x00, 0x00, 0x55, 0x55, 0xd4, 0x01, 0xc0, 0x00, 0x00, 
	0xa0, 0xaa, 0xaa, 0xaa, 0xaa, 0x2a, 0x00, 0x00, 0x20, 0xa0, 0x02, 0xe8, 0x07, 0xa0, 0x00, 0x00, 
	0x50, 0x55, 0x55, 0x55, 0x55, 0x15, 0x00, 0x00, 0x40, 0x05, 0x00, 0xd0, 0x0f, 0xc0, 0x00, 0x00, 
	0xa0, 0xaa, 0xaa, 0xaa, 0xaa, 0x02, 0x00, 0x00, 0x80, 0x0a, 0x00, 0xa8, 0x0f, 0x40, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

struct Enemy {
  float x, y;
  int hp;
  bool chasingPlayer;
  float moveAmount;
  const char* name;
  int attackDelay;
};
const int maxEnemies = 30; // Adjust the number of enemies as needed
Enemy enemies[maxEnemies];

struct Projectile {
  float x, y;
  float dx, dy;
  float speed;
  float damage;
  bool active;
};
const int maxProjectiles = 30;
Projectile projectiles[maxProjectiles];

struct Damsel {
  float x, y;
  float speed;
  bool dead;
  bool followingPlayer;
};
Damsel damsel[1];

int playerDX;
int playerDY;

// SH1107 128x128 SPI Constructor
U8G2_SH1107_PIMORONI_128X128_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RST);

// Timing variables
unsigned long lastUpdateTime = 0;
const unsigned long frameDelay = 20; // Update every 100ms

void setup() {
  Serial.begin(9600);
  u8g2.begin();
  u8g2.setBitmapMode(1);

  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_LEFT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);

  for (int i = 0; i < maxProjectiles; i++) {
      projectiles[i].active = false;
  }

  randomSeed(generateRandomSeed());

  // Generate a random dungeon
  generateDungeon();
  spawnEnemies();
}

void loop() {
  unsigned long currentTime = millis();
  if (playerHP > 0) {
    if (currentTime - lastUpdateTime >= frameDelay) {
      lastUpdateTime = currentTime;

      // Update game state
      handleInput();
      updateScrolling();
      updateDamsel();
      updateEnemies();
      updateProjectiles();

      // Render the game
      u8g2.clearBuffer();
      renderDungeon();
      renderDamsel();
      renderEnemies();
      renderProjectiles();
      renderPlayer();
      updateAnimations();
      renderUI();
      //u8g2.drawXBMP(0, 0, 128, 128, rescueDamselScreen);
      u8g2.sendBuffer();
    }
  }
  else {
    gameOver();
  }
}

int blobanimcounter = 0;
int damselanimcounter = 0;
void updateAnimations() {
  blobanimcounter += 1;
  if (blobanimcounter >= 20) {
    blobSprite = blobSprite == blobSpriteFrame1 ? blobSpriteFrame2 : blobSpriteFrame1;
    blobanimcounter = 0;
  }
  
  damselanimcounter += 1;
  if (damselanimcounter >= random(50, 90)) {
    damselSprite = damsel[0].dead ? damselSpriteDead : damselSprite;
    damselanimcounter = 0;
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

void generateDungeon() {
  // Initialize map with walls
  for (int y = 0; y < mapHeight; y++) {
    for (int x = 0; x < mapWidth; x++) {
      dungeonMap[y][x] = 2; // Wall
    }
  }

  // Room generation parameters
  const int maxRooms = random(15, 25); // Maximum number of rooms
  const int minRoomSize = 4;           // Minimum room size (tiles)
  const int maxRoomSize = 8;           // Maximum room size (tiles)

  struct Room {
    int x, y, width, height;
  };

  Room rooms[maxRooms];
  int roomCount = 0;

  // Guarantee a starting room at the center of the map
  int startRoomWidth = random(minRoomSize, maxRoomSize + 1);
  int startRoomHeight = random(minRoomSize, maxRoomSize + 1);
  int startRoomX = mapWidth / 2 - startRoomWidth / 2;
  int startRoomY = mapHeight / 2 - startRoomHeight / 2;

  // Create the starting room
  rooms[roomCount++] = {startRoomX, startRoomY, startRoomWidth, startRoomHeight};
  for (int y = startRoomY; y < startRoomY + startRoomHeight; y++) {
    for (int x = startRoomX; x < startRoomX + startRoomWidth; x++) {
      dungeonMap[y][x] = 1; // Floor
    }
  }

  // Generate additional rooms
  for (int i = 1; i < maxRooms; i++) {
    int roomWidth = random(minRoomSize, maxRoomSize + 1);
    int roomHeight = random(minRoomSize, maxRoomSize + 1);
    int roomX = random(1, mapWidth - roomWidth - 1);
    int roomY = random(1, mapHeight - roomHeight - 1);

    // Check for overlaps
    bool overlap = false;
    for (int j = 0; j < roomCount; j++) {
      if (roomX < rooms[j].x + rooms[j].width && roomX + roomWidth > rooms[j].x &&
          roomY < rooms[j].y + rooms[j].height && roomY + roomHeight > rooms[j].y) {
        overlap = true;
        break;
      }
    }

    // Add the room if no overlap
    if (!overlap) {
      rooms[roomCount++] = {roomX, roomY, roomWidth, roomHeight};
      for (int y = roomY; y < roomY + roomHeight; y++) {
        for (int x = roomX; x < roomX + roomWidth; x++) {
          dungeonMap[y][x] = 1; // Floor
        }
      }
    }
  }

  // Connect rooms with corridors
  for (int i = 1; i < roomCount; i++) {
    // Get the center of the current room and the previous room
    int x1 = rooms[i - 1].x + rooms[i - 1].width / 2;
    int y1 = rooms[i - 1].y + rooms[i - 1].height / 2;
    int x2 = rooms[i].x + rooms[i].width / 2;
    int y2 = rooms[i].y + rooms[i].height / 2;

    // Randomly decide corridor order (horizontal-first or vertical-first)
    if (random(0, 2) == 0) {
      carveHorizontalCorridor(x1, x2, y1);
      carveVerticalCorridor(y1, y2, x2);
    } else {
      carveVerticalCorridor(y1, y2, x1);
      carveHorizontalCorridor(x1, x2, y2);
    }
  }

  // This is to get rid of any lone tiles
  for (int y = 0; y < mapHeight; y++) {
    for (int x = 0; x < mapWidth; x++) {
      if (dungeonMap[y][x] == 2) {
        int neighbors = 0;

        for(int nx = -1; nx < 2; nx++) {
          for(int ny = -1; ny < 2; ny++) {
            if (nx == 0 && ny == 0) continue;
            if (dungeonMap[y + ny][x + nx] == 2) {
              neighbors += 1;
            }
          }
        }

        if (neighbors <= 1) {
          dungeonMap[y][x] = 1;
        }
      }
    }
  }

  // Generate the damsel's cell
  int damselRoomWidth = 7;
  int damselRoomHeight = 5;
  int damselRoomX, damselRoomY;

  // Find a location far from the start room
  do {
    damselRoomX = random(1, mapWidth - damselRoomWidth - 1);
    damselRoomY = random(1, mapHeight - damselRoomHeight - 1);
  } while (abs(damselRoomX - startRoomX) + abs(damselRoomY - startRoomY) < mapWidth / 2);

  // Create the damsel's cell
  for (int y = damselRoomY; y < damselRoomY + damselRoomHeight; y++) {
    for (int x = damselRoomX; x < damselRoomX + damselRoomWidth; x++) {
      if ((y == damselRoomY || y == damselRoomY + damselRoomHeight - 1) && x >= damselRoomX && x < damselRoomX + damselRoomWidth) {
        dungeonMap[y][x] = 3; // Walls using barsSprite
      } else {
        dungeonMap[y][x] = 1; // Floor
      }
    }
  }

  // Connect the damsel's cell to the dungeon
  int centerX = damselRoomX + damselRoomWidth / 2;
  int centerY = damselRoomY + damselRoomHeight / 2;
  int startCenterX = startRoomX + startRoomWidth / 2;
  int startCenterY = startRoomY + startRoomHeight / 2;

  carveHorizontalCorridor(startCenterX, centerX, startCenterY);
  carveVerticalCorridor(startCenterY, centerY, centerX);

  // Place the damsel in the cell
  damsel[0].x = centerX-1;
  damsel[0].y = centerY-1;
  damsel[0].speed = 0.1;
  damsel[0].followingPlayer = false;
  damsel[0].dead = false;

  dungeonMap[startRoomX + (startRoomWidth/2)][startRoomY + (startRoomHeight/2) + 1] = 0;

  // Ensure player start
  int playerStartX = startRoomX + startRoomWidth / 2;
  int playerStartY = startRoomY + startRoomHeight / 2;
  dungeonMap[playerStartY][playerStartX] = 1; // Make sure the player's position is a floor
  playerX = playerStartX;
  playerY = playerStartY;

  // Place the exit in the last room
  dungeonMap[rooms[roomCount - 1].y + rooms[roomCount - 1].height / 2]
            [rooms[roomCount - 1].x + rooms[roomCount - 1].width / 2] = 4; // Exit
}

// Carve a horizontal corridor
void carveHorizontalCorridor(int x1, int x2, int y) {
  if (x1 > x2) swap(x1, x2);
  for (int x = x1; x <= x2; x++) {
    dungeonMap[y][x] = 1; // Floor
  }
}

// Carve a vertical corridor
void carveVerticalCorridor(int y1, int y2, int x) {
  if (y1 > y2) swap(y1, y2);
  for (int y = y1; y <= y2; y++) {
    dungeonMap[y][x] = 1; // Floor
  }
}

// Utility function to swap values
void swap(int &a, int &b) {
  int temp = a;
  a = b;
  b = temp;
}

// Count surrounding walls for smoothing
int countWalls(int x, int y) {
  int wallCount = 0;
  for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {
      if (dx != 0 || dy != 0) {
        if (dungeonMap[y + dy][x + dx] == 2) {
          wallCount++;
        }
      }
    }
  }
  return wallCount;
}

// Render the visible portion of the dungeon
void renderDungeon() {
  for (int y = 0; y < viewportHeight + 1; y++) { // +1 to handle partial tiles at edges
    for (int x = 0; x < viewportWidth + 1; x++) {
      float mapX = x + offsetX;
      float mapY = y + offsetY;

      if (mapX >= 0 && mapX < mapWidth && mapY >= 0 && mapY < mapHeight) {
        // Calculate screen position based on fractional offsets
        float screenX = (x - (offsetX - (int)offsetX)) * tileSize;
        float screenY = (y - (offsetY - (int)offsetY)) * tileSize;

        drawTile((int)mapX, (int)mapY, screenX, screenY);
      }
    }
  }
}

// Draw a tile based on its type
void drawTile(int mapX, int mapY, float screenX, float screenY) {
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
    case 3: // Bars
      u8g2.drawXBMP(screenX, screenY, tileSize, tileSize, barsSprite);
      break;
    case 4: // Exit
      u8g2.drawXBMP(screenX, screenY, tileSize, tileSize, stairsSprite);
      break;
  }
}

// Render the player
void renderPlayer() {
  float screenX = (playerX - offsetX) * tileSize;
  float screenY = (playerY - offsetY) * tileSize;

  // Ensure the player is within the viewport
  if (screenX >= 0 && screenX < 128 && screenY >= 0 && screenY < 128) {
    //u8g2.drawDisc(screenX + tileSize / 2, screenY + tileSize / 2, tileSize / 3, U8G2_DRAW_ALL);
    u8g2.drawXBMP((screenX + tileSize / 2) - tileSize/2, (screenY + tileSize / 2) - tileSize/2, tileSize, tileSize, playerSprite);
  }
}

void spawnEnemies() {
  for (int i = 0; i < maxEnemies; i++) {
    while (true) {
      int ex = random(0, mapWidth);
      int ey = random(0, mapHeight);
      if (dungeonMap[ey][ex] == 1) { // Only spawn on floor tiles
        enemies[i] = {(float)ex, (float)ey, 20, false, 0.05, "blob", 20};
        break;
      }
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

int predictXtile(float x) {
  return (int)(x + 0.5f); // Always round to the nearest integer
}

int predictYtile(float y) {
  return (int)(y + 0.5f); // Always round to the nearest integer
}

bool checkSpriteCollisionWithTileX(float newX, float currentX, float newY) {
    int ptx = predictXtile(newX);
    int cty = round(newY);

    bool xValid = (newX >= 0 && newX < mapWidth && dungeonMap[cty][ptx] == 1);

    if (!xValid) {
        newX = currentX;
    }

    return !xValid;
}

bool checkSpriteCollisionWithTileY(float newY, float currentY, float newX) {
    int pty = predictYtile(newY);
    int ctx = round(newX);

    bool yValid = (newY >= 0 && newY < mapHeight && dungeonMap[pty][ctx] == 1);

    if (!yValid) {
        newY = currentY;
    }

    return !yValid;
}

bool checkSpriteCollisionWithSprite(float sprite1X, float sprite1Y, float sprite2X, float sprite2Y) {
  // Use predictXtile/predictYtile for consistent rounding
  int tile1X = predictXtile(sprite1X);
  int tile1Y = predictYtile(sprite1Y);
  int tile2X = predictXtile(sprite2X);
  int tile2Y = predictYtile(sprite2Y);

  return tile1X == tile2X && tile1Y == tile2Y;
}

void renderEnemies() {
  for (int i = 0; i < maxEnemies; i++) {
    if (enemies[i].hp > 0) {
      float screenX = (enemies[i].x - offsetX) * tileSize;
      float screenY = (enemies[i].y - offsetY) * tileSize;
      if (screenX >= 0 && screenY >= 0 && screenX < 128 && screenY < 128) {
        u8g2.drawXBMP(screenX, screenY, 8, 8, blobSprite);
      }
    }
  }
}

int damselMoveDelay = 0;
void updateDamsel() {
  if (!damsel[0].dead) {
    damselMoveDelay++;
  }

  float destinationX;
  float destinationY;

  destinationX = playerDX == 1 ? playerX - 1 : playerDX == -1 ? playerX + 1 : playerX;
  destinationY = playerDY == 1 ? playerY - 1 : playerDY == -1 ? playerY + 1 : playerY;

  // Calculate distance to player
  int dx = round(destinationX) - round(damsel[0].x);
  int dy = round(destinationY) - round(damsel[0].y);
  int distanceSquared = dx * dx + dy * dy;

  // Check if the damsel should follow the player
  if (distanceSquared <= 25) { // Follow if within 5 tiles (distance^2 = 25)
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

void renderDamsel() {
  float screenX = (damsel[0].x - offsetX) * tileSize;
  float screenY = (damsel[0].y - offsetY) * tileSize;
  if (screenX >= 0 && screenY >= 0 && screenX < 128 && screenY < 128) {
    u8g2.drawXBMP(screenX, screenY, 8, 8, damselSprite);
  }
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
        if (checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y, enemies[j].x, enemies[j].y) && enemies[j].hp > 0) {
          enemies[j].hp -= projectiles[i].damage;    // Reduce enemy health
          if (enemies[j].hp <= 0 && projectiles[i].active == true) {
            kills += 1;
          }
          projectiles[i].active = false; // Deactivate the bullet
        } else if (!damsel[0].dead && checkSpriteCollisionWithSprite(projectiles[i].x, projectiles[i].y, damsel[0].x, damsel[0].y)) {
          damsel[0].dead = true;
          projectiles[i].active = false;
        }
      }
    }
  }
}

void renderProjectiles() {
    for (int i = 0; i < maxProjectiles; i++) {
        if (projectiles[i].active) {
          float screenX = (projectiles[i].x - offsetX) * tileSize + tileSize/2;
          float screenY = (projectiles[i].y - offsetY) * tileSize + tileSize/2;
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

int shootDelay = 0;
bool reloading;
void handleInput() {
  float newX = playerX;
  float newY = playerY;

  // Read button states (inverted because of pull-up resistors)
  bool upPressed = !digitalRead(BUTTON_UP_PIN);
  bool downPressed = !digitalRead(BUTTON_DOWN_PIN);
  bool leftPressed = !digitalRead(BUTTON_LEFT_PIN);
  bool rightPressed = !digitalRead(BUTTON_RIGHT_PIN);
  bool bPressed = !digitalRead(BUTTON_B_PIN);
  //bool aPressed = !digitalRead(BUTTON_A_PIN);

  if (upPressed) {
    playerDY = -1;
    playerDX = 0;
    newY -= 0.1; // Move up
  } else if (downPressed) {
    playerDY = 1;
    playerDX = 0;
    newY += 0.1; // Move down
  } else if (leftPressed) {
    playerDX = -1;
    playerDY = 0;
    playerSprite = playerSpriteLeft;
    newX -= 0.1; // Move left
  } else if (rightPressed) {
    playerDX = 1;
    playerDY = 0;
    playerSprite = playerSpriteRight;
    newX += 0.1; // Move right
  }

  if (bPressed && !reloading) {
    shootProjectile(playerDX, playerDY); // Shoot in current direction
    reloading = true;
  }

  if (reloading) {
    shootDelay++;
    if (shootDelay >= 10) {
      reloading = false;
      shootDelay = 0;
    }
  }

  int rNewX = round(newX);
  int rNewY = round(newY);

  // Check collision with walls
  if (dungeonMap[rNewY][rNewX] == 1 || dungeonMap[rNewY][rNewX] == 4 || dungeonMap[rNewY][rNewX] == 0) {
    playerX = newX;
    playerY = newY;

    // Update viewport offset if needed
    if (playerX - offsetX < 2 && offsetX > 0) offsetX -= scrollSpeed;
    if (playerX - offsetX > viewportWidth - 3 && offsetX < mapWidth - viewportWidth) offsetX += scrollSpeed;
    if (playerY - offsetY < 2 && offsetY > 0) offsetY -= scrollSpeed;
    if (playerY - offsetY > viewportHeight - 3 && offsetY < mapHeight - viewportHeight) offsetY += scrollSpeed;
  }

  int rPx = round(playerX);
  int rPy = round(playerY);

  // Check if the player reached the exit
  if (dungeonMap[rPy][rPx] == 4) {
    Serial.println("You reached the exit!");
    level += 1;
    generateDungeon(); // Generate a new dungeon
    spawnEnemies();
  }
}

bool pressed;
int page = 1;
int pageDelay = 0;
void gameOver() {
  bool bPressed = !digitalRead(BUTTON_B_PIN);
  bool aPressed = !digitalRead(BUTTON_A_PIN);

  if (aPressed && !pressed) {
    page++;
    if (page == 3) {
      page = 1;
    }
    pressed = true;
  }

  if (!aPressed) {
    pageDelay = 0;
    pressed = false;
  }

  if (pressed) {
    pageDelay++;
    if (pageDelay >= 50) {
      pageDelay = 0;
      pressed = false;
    }
  }

  char Lvl[7];
  snprintf(Lvl, sizeof(Lvl), "%d", level);
  char KLLS[7];
  snprintf(KLLS, sizeof(KLLS), "%d", kills);
  
  int lvlHighscore;
  lvlHighscore = EEPROM.read(lvlHighscoreAddress);
  if (level > lvlHighscore) {
    EEPROM.write(lvlHighscoreAddress, level);
  }

  int kllHighscore;
  kllHighscore = EEPROM.read(killHighscoreAddress);
  if (kills > kllHighscore) {
    EEPROM.write(killHighscoreAddress, kills);
  }

  char LHighscore[7];
  snprintf(LHighscore, sizeof(LHighscore), "%d", lvlHighscore);
  char KHighscore[7];
  snprintf(KHighscore, sizeof(KHighscore), "%d", kllHighscore);

  u8g2.clearBuffer();
  //Serial.println("You died!");
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(11, 30, "Game over!");

  u8g2.drawFrame(11, 42, 108, 80);

  u8g2.setFont(u8g2_font_profont12_tr);
  if (page == 1) {
    u8g2.drawStr(15, 54, "Slain by:");
    u8g2.drawStr(70, 54, deathCause);

    u8g2.drawStr(15, 66, "On level:");
    u8g2.drawStr(70, 66, Lvl);

    u8g2.drawStr(15, 78, "Lvl highscore:");
    u8g2.drawStr(100, 78, LHighscore);

    u8g2.drawStr(15, 90, "Kills:");
    u8g2.drawStr(52, 90, KLLS);

    u8g2.drawStr(15, 102, "Kll Highscore:");
    u8g2.drawStr(100, 102, KHighscore);

    u8g2.drawStr(15, 114, "[A] next page");
  } else if (page == 2) {
    u8g2.drawStr(15, 54, "next page");

    u8g2.drawStr(15, 114, "[A] next page");
  }

  u8g2.sendBuffer();
  if (bPressed) {
    playerHP = 100;
    level = 1;
    generateDungeon();
    spawnEnemies();
  }
}

uint32_t generateRandomSeed()
{
  uint8_t  seedBitValue  = 0;
  uint8_t  seedByteValue = 0;
  uint32_t seedWordValue = 0;
 
  for (uint8_t wordShift = 0; wordShift < 4; wordShift++)     // 4 bytes in a 32 bit word
  {
    for (uint8_t byteShift = 0; byteShift < 8; byteShift++)   // 8 bits in a byte
    {
      for (uint8_t bitSum = 0; bitSum <= 8; bitSum++)         // 8 samples of analog pin
      {
        seedBitValue = seedBitValue + (analogRead(seedPin) & 0x01);                // Flip the coin eight times, adding the results together
      }
      delay(1);                                                                    // Delay a single millisecond to allow the pin to fluctuate
      seedByteValue = seedByteValue | ((seedBitValue & 0x01) << byteShift);        // Build a stack of eight flipped coins
      seedBitValue = 0;                                                            // Clear out the previous coin value
    }
    seedWordValue = seedWordValue | (uint32_t)seedByteValue << (8 * wordShift);    // Build a stack of four sets of 8 coins (shifting right creates a larger number so cast to 32bit)
    seedByteValue = 0;                                                             // Clear out the previous stack value
  }
  return (seedWordValue);
 
}
