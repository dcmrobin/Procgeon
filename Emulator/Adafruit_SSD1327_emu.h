#pragma once
#include "Adafruit_GFX_emu.h"

class Adafruit_SSD1327 : public Adafruit_GFX {
public:
    Adafruit_SSD1327(int8_t rst_pin = -1);
    ~Adafruit_SSD1327();
    
    bool begin(uint8_t i2c_addr = 0x3C);
    void display();
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    
    uint8_t* getBuffer();
    void clearDisplay();

private:
    uint8_t* _buffer;
};

#define SSD1327_BLACK  0
#define SSD1327_WHITE  15
#define SSD1327_GRAY   7