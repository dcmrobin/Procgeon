#include "Adafruit_SSD1327_emu.h"
#include <cstring>

Adafruit_SSD1327::Adafruit_SSD1327(int8_t /* rst_pin */) 
  : Adafruit_GFX(128, 128), _rotation(0) {
    _buffer = new uint8_t[128 * 128]();
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
    if (x < 0 || x >= _width || y < 0 || y >= _height) return;
    
    switch (_rotation) {
        case 1:
            std::swap(x, y);
            x = 128 - x - 1;
            break;
        case 2:
            x = 128 - x - 1;
            y = 128 - y - 1;
            break;
        case 3:
            std::swap(x, y);
            y = 128 - y - 1;
            break;
    }
    
    if (x >= 0 && x < 128 && y >= 0 && y < 128) {
        _buffer[y * 128 + x] = color & 0x0F;
    }
}

void Adafruit_SSD1327::setRotation(uint8_t r) {
    _rotation = r % 4;
}

// Dummy implementations for compatibility
void Adafruit_SSD1327::dim(bool /* dim */) {}
void Adafruit_SSD1327::invertDisplay(bool /* invert */) {}
void Adafruit_SSD1327::setContrast(uint8_t /* contrast */) {}

uint8_t* Adafruit_SSD1327::getBuffer() {
    return _buffer;
}