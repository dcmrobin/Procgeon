#pragma once
#include <cstdint>

class Adafruit_GFX {
public:
    Adafruit_GFX(int16_t w, int16_t h) : _width(w), _height(h) {}
    
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    
    // Basic drawing functions
    void fillScreen(uint16_t color) {
        for (int16_t y = 0; y < _height; y++) {
            for (int16_t x = 0; x < _width; x++) {
                drawPixel(x, y, color);
            }
        }
    }
    
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
        for (int16_t i = x; i < x + w; i++) {
            drawPixel(i, y, color);
            drawPixel(i, y + h - 1, color);
        }
        for (int16_t i = y; i < y + h; i++) {
            drawPixel(x, i, color);
            drawPixel(x + w - 1, i, color);
        }
    }
    
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
        for (int16_t i = y; i < y + h; i++) {
            for (int16_t j = x; j < x + w; j++) {
                drawPixel(j, i, color);
            }
        }
    }
    
    // Add more drawing functions as needed (drawLine, drawCircle, etc.)

protected:
    int16_t _width, _height;
};