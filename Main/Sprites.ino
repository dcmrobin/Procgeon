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