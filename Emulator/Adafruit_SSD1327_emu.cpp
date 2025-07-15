#include "Adafruit_SSD1327_emu.h"
#include <cstring>

Adafruit_SSD1327::Adafruit_SSD1327(int8_t /* rst_pin */) 
  : Adafruit_GFX(128, 128) {
    _buffer = new uint8_t[128 * 128](); // Zero-initialize
}

Adafruit_SSD1327::~Adafruit_SSD1327() {
    delete[] _buffer;
}

bool Adafruit_SSD1327::begin(uint8_t /* i2c_addr */) {
    return true;
}

void Adafruit_SSD1327::display() {
    // No hardware refresh needed
}

void Adafruit_SSD1327::clearDisplay() {
    memset(_buffer, 0, 128 * 128);
}

void Adafruit_SSD1327::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x >= 0 && x < _width && y >= 0 && y < _height) {
        _buffer[y * _width + x] = color & 0x0F;
    }
}

uint8_t* Adafruit_SSD1327::getBuffer() {
    return _buffer;
}