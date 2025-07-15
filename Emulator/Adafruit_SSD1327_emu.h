#pragma once
#include "Adafruit_GFX_emu.h"

class Adafruit_SSD1327 : public Adafruit_GFX {
public:
    Adafruit_SSD1327(int8_t rst_pin = -1);
    ~Adafruit_SSD1327();
    
    bool begin(uint8_t i2c_addr = 0x3C);
    void display();
    void clearDisplay();
    
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    
    // SSD1327-specific functions
    void dim(bool dim);
    void invertDisplay(bool invert);
    void setContrast(uint8_t contrast);
    void setRotation(uint8_t r);
    
    uint8_t* getBuffer();

    // Color definitions
    static const uint16_t SSD1327_BLACK = 0;
    static const uint16_t SSD1327_WHITE = 15;
    static const uint16_t SSD1327_LIGHTGRAY = 10;
    static const uint16_t SSD1327_GRAY = 7;
    static const uint16_t SSD1327_DARKGRAY = 3;
    
private:
    uint8_t* _buffer;
    uint8_t _rotation;
};