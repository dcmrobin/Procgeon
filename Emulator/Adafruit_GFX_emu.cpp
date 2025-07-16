#include "Adafruit_GFX_emu.h"
#include <algorithm>
#include <cstring>

#ifndef __AVR__
// Emulate PROGMEM for non-Arduino platforms
#define pgm_read_byte(addr) (*(const uint8_t *)(addr))
#endif

// Add fontWidth and fontHeight member initialization
Adafruit_GFX::Adafruit_GFX(int16_t w, int16_t h)
    : _width(w), _height(h), fontWidth(5), fontHeight(7) {
    // Ensure default font is set
    font = builtin_font;
}

// Drawing functions
void Adafruit_GFX::startWrite() {}
void Adafruit_GFX::endWrite() {}

void Adafruit_GFX::writePixel(int16_t x, int16_t y, uint16_t color) {
    drawPixel(x, y, color);
}

void Adafruit_GFX::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (w < 0) { x += w; w = -w; }
    if (h < 0) { y += h; h = -h; }
    
    for (int16_t i = y; i < y + h; i++) {
        for (int16_t j = x; j < x + w; j++) {
            if (j >= 0 && j < _width && i >= 0 && i < _height) {
                drawPixel(j, i, color);
            }
        }
    }
}

void Adafruit_GFX::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    for (int16_t i = y; i < y + h; i++) {
        if (x >= 0 && x < _width && i >= 0 && i < _height) {
            drawPixel(x, i, color);
        }
    }
}

void Adafruit_GFX::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    for (int16_t i = x; i < x + w; i++) {
        if (i >= 0 && i < _width && y >= 0 && y < _height) {
            drawPixel(i, y, color);
        }
    }
}

void Adafruit_GFX::fillScreen(uint16_t color) {
    writeFillRect(0, 0, _width, _height, color);
}

void Adafruit_GFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    startWrite();
    writeFillRect(x, y, w, h, color);
    endWrite();
}

void Adafruit_GFX::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    startWrite();
    writeFastHLine(x, y, w, color);
    writeFastHLine(x, y + h - 1, w, color);
    writeFastVLine(x, y, h, color);
    writeFastVLine(x + w - 1, y, h, color);
    endWrite();
}

void Adafruit_GFX::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    startWrite();
    writePixel(x0, y0 + r, color);
    writePixel(x0, y0 - r, color);
    writePixel(x0 + r, y0, color);
    writePixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        
        writePixel(x0 + x, y0 + y, color);
        writePixel(x0 - x, y0 + y, color);
        writePixel(x0 + x, y0 - y, color);
        writePixel(x0 - x, y0 - y, color);
        writePixel(x0 + y, y0 + x, color);
        writePixel(x0 - y, y0 + x, color);
        writePixel(x0 + y, y0 - x, color);
        writePixel(x0 - y, y0 - x, color);
    }
    endWrite();
}

void Adafruit_GFX::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    startWrite();
    writeFastVLine(x0, y0 - r, 2 * r + 1, color);
    fillCircleHelper(x0, y0, r, 3, 0, color);
    endWrite();
}

void Adafruit_GFX::drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    startWrite();
    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        if (cornername & 0x4) {
            writePixel(x0 + x, y0 + y, color);
            writePixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            writePixel(x0 + x, y0 - y, color);
            writePixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            writePixel(x0 - y, y0 + x, color);
            writePixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            writePixel(x0 - y, y0 - x, color);
            writePixel(x0 - x, y0 - y, color);
        }
    }
    endWrite();
}

void Adafruit_GFX::fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    delta++;
    
    startWrite();
    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (cornername & 0x1) {
            writeFastVLine(x0 + x, y0 - y, 2 * y + delta, color);
            writeFastVLine(x0 + y, y0 - x, 2 * x + delta, color);
        }
        if (cornername & 0x2) {
            writeFastVLine(x0 - x, y0 - y, 2 * y + delta, color);
            writeFastVLine(x0 - y, y0 - x, 2 * x + delta, color);
        }
    }
    endWrite();
}

void Adafruit_GFX::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color) {
    // Sanity check radius
    int16_t maxRadius = std::min(w, h) / 2;
    radius = std::min(radius, maxRadius);
    
    startWrite();
    // Draw sides
    writeFastHLine(x + radius, y, w - 2 * radius, color);         // Top
    writeFastHLine(x + radius, y + h - 1, w - 2 * radius, color); // Bottom
    writeFastVLine(x, y + radius, h - 2 * radius, color);         // Left
    writeFastVLine(x + w - 1, y + radius, h - 2 * radius, color); // Right
    
    // Draw four corners
    drawCircleHelper(x + radius, y + radius, radius, 1, color);
    drawCircleHelper(x + w - radius - 1, y + radius, radius, 2, color);
    drawCircleHelper(x + w - radius - 1, y + h - radius - 1, radius, 4, color);
    drawCircleHelper(x + radius, y + h - radius - 1, radius, 8, color);
    endWrite();
}

void Adafruit_GFX::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color) {
    // Sanity check radius
    int16_t maxRadius = std::min(w, h) / 2;
    radius = std::min(radius, maxRadius);
    
    startWrite();
    // Draw center rectangle
    writeFillRect(x + radius, y, w - 2 * radius, h, color);
    
    // Draw four corners
    fillCircleHelper(x + w - radius - 1, y + radius, radius, 1, h - 2 * radius - 1, color);
    fillCircleHelper(x + radius, y + radius, radius, 2, h - 2 * radius - 1, color);
    endWrite();
}

void Adafruit_GFX::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    startWrite();
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
    endWrite();
}

void Adafruit_GFX::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    int16_t a, b, y, last;
    
    // Sort coordinates by Y order (y0 <= y1 <= y2)
    if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }
    if (y1 > y2) {
        std::swap(y2, y1);
        std::swap(x2, x1);
    }
    if (y0 > y1) {
        std::swap(y0, y1);
        std::swap(x0, x1);
    }
    
    startWrite();
    
    // Handle degenerate triangles
    if (y0 == y2) {
        a = b = x0;
        if (x1 < a) a = x1;
        else if (x1 > b) b = x1;
        if (x2 < a) a = x2;
        else if (x2 > b) b = x2;
        writeFastHLine(a, y0, b - a + 1, color);
        endWrite();
        return;
    }
    
    int16_t dx01 = x1 - x0, dy01 = y1 - y0;
    int16_t dx02 = x2 - x0, dy02 = y2 - y0;
    int16_t dx12 = x2 - x1, dy12 = y2 - y1;
    int32_t sa = 0, sb = 0;
    
    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2. If y1=y2, scanline y1 is included here (and second
    // loop will be skipped, avoiding a /0 error there)
    if (y1 == y2) last = y1;  // Include y1 scanline
    else last = y1 - 1;       // Skip it
    
    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        if (a > b) std::swap(a, b);
        writeFastHLine(a, y, b - a + 1, color);
    }
    
    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2
    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        if (a > b) std::swap(a, b);
        writeFastHLine(a, y, b - a + 1, color);
    }
    
    endWrite();
}

void Adafruit_GFX::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int16_t dx = x1 - x0;
    int16_t dy = abs(y1 - y0);
    int16_t err = dx / 2;
    int16_t ystep = (y0 < y1) ? 1 : -1;

    startWrite();
    for (; x0 <= x1; x0++) {
        if (steep) {
            writePixel(y0, x0, color);
        } else {
            writePixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
    endWrite();
}

void Adafruit_GFX::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    startWrite();
    writeFastVLine(x, y, h, color);
    endWrite();
}

void Adafruit_GFX::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    startWrite();
    writeFastHLine(x, y, w, color);
    endWrite();
}

// Text functions
void Adafruit_GFX::setTextSize(uint8_t s) { textsize = (s > 0) ? s : 1; }
void Adafruit_GFX::setTextColor(uint16_t c) { textcolor = textbgcolor = c; }
void Adafruit_GFX::setTextColor(uint16_t c, uint16_t b) { textcolor = c; textbgcolor = b; }
void Adafruit_GFX::setTextWrap(bool w) { wrap = w; }
void Adafruit_GFX::setCursor(int16_t x, int16_t y) { cursor_x = x; cursor_y = y; }

size_t Adafruit_GFX::print(char c) { 
    return print(std::string(1, c)); 
}

// Compact 4x6 pixel ASCII font (printable 32-126)
// Each character is 4 bytes (columns), 6 bits used per byte (rows, LSB at top)
const uint8_t Adafruit_GFX::profont10_font[] = {
#define PF4X6(a,b,c,d) a,b,c,d
    PF4X6(0x00,0x00,0x00,0x00), // ' '
    PF4X6(0x00,0x2F,0x00,0x00), // '!'
    PF4X6(0x03,0x00,0x03,0x00), // '"'
    PF4X6(0x3E,0x14,0x3E,0x14), // '#'
    PF4X6(0x14,0x3F,0x15,0x0A), // '$'
    PF4X6(0x33,0x08,0x04,0x33), // '%'
    PF4X6(0x1A,0x25,0x2A,0x10), // '&'
    PF4X6(0x00,0x03,0x00,0x00), // '''
    PF4X6(0x0E,0x11,0x00,0x00), // '('
    PF4X6(0x00,0x00,0x11,0x0E), // ')'
    PF4X6(0x0A,0x04,0x0A,0x00), // '*'
    PF4X6(0x04,0x0E,0x04,0x00), // '+' 
    PF4X6(0x10,0x08,0x00,0x00), // ','
    PF4X6(0x04,0x04,0x04,0x00), // '-'
    PF4X6(0x00,0x10,0x00,0x00), // '.'
    PF4X6(0x30,0x08,0x04,0x03), // '/'
    PF4X6(0x1E,0x21,0x21,0x1E), // '0'
    PF4X6(0x00,0x22,0x3F,0x20), // '1'
    PF4X6(0x32,0x29,0x29,0x26), // '2'
    PF4X6(0x12,0x21,0x25,0x1A), // '3'
    PF4X6(0x0C,0x0A,0x09,0x3F), // '4'
    PF4X6(0x17,0x25,0x25,0x19), // '5'
    PF4X6(0x1E,0x25,0x25,0x18), // '6'
    PF4X6(0x01,0x39,0x05,0x03), // '7'
    PF4X6(0x1A,0x25,0x25,0x1A), // '8'
    PF4X6(0x06,0x29,0x29,0x1E), // '9'
    PF4X6(0x00,0x14,0x00,0x00), // ':'
    PF4X6(0x10,0x14,0x00,0x00), // ';'
    PF4X6(0x04,0x0A,0x11,0x00), // '<'
    PF4X6(0x0A,0x0A,0x0A,0x0A), // '='
    PF4X6(0x00,0x11,0x0A,0x04), // '>'
    PF4X6(0x02,0x29,0x05,0x02), // '?'
    PF4X6(0x1E,0x29,0x2D,0x0E), // '@'
    PF4X6(0x3E,0x09,0x09,0x3E), // 'A'
    PF4X6(0x3F,0x25,0x25,0x1A), // 'B'
    PF4X6(0x1E,0x21,0x21,0x12), // 'C'
    PF4X6(0x3F,0x21,0x21,0x1E), // 'D'
    PF4X6(0x3F,0x25,0x25,0x21), // 'E'
    PF4X6(0x3F,0x09,0x09,0x01), // 'F'
    PF4X6(0x1E,0x21,0x29,0x3A), // 'G'
    PF4X6(0x3F,0x08,0x08,0x3F), // 'H'
    PF4X6(0x00,0x21,0x3F,0x21), // 'I'
    PF4X6(0x10,0x20,0x21,0x1F), // 'J'
    PF4X6(0x3F,0x08,0x14,0x23), // 'K'
    PF4X6(0x3F,0x20,0x20,0x20), // 'L'
    PF4X6(0x3F,0x02,0x04,0x3F), // 'M'
    PF4X6(0x3F,0x04,0x08,0x3F), // 'N'
    PF4X6(0x1E,0x21,0x21,0x1E), // 'O'
    PF4X6(0x3F,0x09,0x09,0x06), // 'P'
    PF4X6(0x1E,0x21,0x31,0x3E), // 'Q'
    PF4X6(0x3F,0x09,0x19,0x26), // 'R'
    PF4X6(0x26,0x29,0x29,0x12), // 'S'
    PF4X6(0x01,0x01,0x3F,0x01), // 'T'
    PF4X6(0x1F,0x20,0x20,0x1F), // 'U'
    PF4X6(0x0F,0x10,0x20,0x1F), // 'V'
    PF4X6(0x3F,0x10,0x08,0x3F), // 'W'
    PF4X6(0x33,0x0C,0x0C,0x33), // 'X'
    PF4X6(0x03,0x04,0x38,0x07), // 'Y'
    PF4X6(0x31,0x29,0x25,0x23), // 'Z'
    PF4X6(0x00,0x3F,0x21,0x00), // '['
    PF4X6(0x03,0x04,0x08,0x30), // '\\'
    PF4X6(0x00,0x21,0x3F,0x00), // ']'
    PF4X6(0x02,0x01,0x02,0x00), // '^
    PF4X6(0x20,0x20,0x20,0x20), // '_'
    PF4X6(0x01,0x02,0x00,0x00), // '`'
    PF4X6(0x18,0x24,0x24,0x3C), // 'a'
    PF4X6(0x3F,0x24,0x24,0x18), // 'b'
    PF4X6(0x18,0x24,0x24,0x24), // 'c'
    PF4X6(0x18,0x24,0x24,0x3F), // 'd'
    PF4X6(0x18,0x2C,0x2C,0x28), // 'e'
    PF4X6(0x04,0x3E,0x05,0x00), // 'f'
    PF4X6(0x38,0x24,0x24,0x3C), // 'g'
    PF4X6(0x3F,0x04,0x04,0x38), // 'h'
    PF4X6(0x00,0x3D,0x00,0x00), // 'i'
    PF4X6(0x20,0x40,0x44,0x3D), // 'j'
    PF4X6(0x3F,0x08,0x14,0x20), // 'k'
    PF4X6(0x00,0x3F,0x20,0x00), // 'l'
    PF4X6(0x3C,0x04,0x18,0x3C), // 'm'
    PF4X6(0x3C,0x04,0x04,0x38), // 'n'
    PF4X6(0x18,0x24,0x24,0x18), // 'o'
    PF4X6(0x3C,0x14,0x14,0x08), // 'p'
    PF4X6(0x08,0x14,0x14,0x3C), // 'q'
    PF4X6(0x3C,0x08,0x04,0x08), // 'r'
    PF4X6(0x28,0x2C,0x34,0x14), // 's'
    PF4X6(0x04,0x1F,0x24,0x20), // 't'
    PF4X6(0x1C,0x20,0x20,0x3C), // 'u'
    PF4X6(0x0C,0x10,0x20,0x1C), // 'v'
    PF4X6(0x3C,0x10,0x08,0x3C), // 'w'
    PF4X6(0x24,0x18,0x18,0x24), // 'x'
    PF4X6(0x0C,0x20,0x20,0x1C), // 'y'
    PF4X6(0x34,0x2C,0x24,0x00), // 'z'
    PF4X6(0x04,0x1B,0x21,0x00), // '{'
    PF4X6(0x00,0x3F,0x00,0x00), // '|'
    PF4X6(0x00,0x21,0x1B,0x04), // '}'
    PF4X6(0x08,0x1C,0x08,0x00), // '->'
    PF4X6(0x08,0x1C,0x08,0x00)  // '<-'
#undef PF4X6
};

// Simple 5x7 font data
const uint8_t Adafruit_GFX::builtin_font[] = {
    0x00, 0x00, 0x00, 0x00, 0x00,  // (space)
    0x00, 0x00, 0x5F, 0x00, 0x00,  // !
    0x00, 0x07, 0x00, 0x07, 0x00,  // "
    0x14, 0x7F, 0x14, 0x7F, 0x14,  // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12,  // $
    0x23, 0x13, 0x08, 0x64, 0x62,  // %
    0x36, 0x49, 0x55, 0x22, 0x50,  // &
    0x00, 0x05, 0x03, 0x00, 0x00,  // '
    0x00, 0x1C, 0x22, 0x41, 0x00,  // (
    0x00, 0x41, 0x22, 0x1C, 0x00,  // )
    0x08, 0x2A, 0x1C, 0x2A, 0x08,  // *
    0x08, 0x08, 0x3E, 0x08, 0x08,  // +
    0x00, 0x50, 0x30, 0x00, 0x00,  // ,
    0x08, 0x08, 0x08, 0x08, 0x08,  // -
    0x00, 0x60, 0x60, 0x00, 0x00,  // .
    0x20, 0x10, 0x08, 0x04, 0x02,  // /
    0x3E, 0x51, 0x49, 0x45, 0x3E,  // 0
    0x00, 0x42, 0x7F, 0x40, 0x00,  // 1
    0x42, 0x61, 0x51, 0x49, 0x46,  // 2
    0x21, 0x41, 0x45, 0x4B, 0x31,  // 3
    0x18, 0x14, 0x12, 0x7F, 0x10,  // 4
    0x27, 0x45, 0x45, 0x45, 0x39,  // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30,  // 6
    0x01, 0x71, 0x09, 0x05, 0x03,  // 7
    0x36, 0x49, 0x49, 0x49, 0x36,  // 8
    0x06, 0x49, 0x49, 0x29, 0x1E,  // 9
    0x00, 0x36, 0x36, 0x00, 0x00,  // :
    0x00, 0x56, 0x36, 0x00, 0x00,  // ;
    0x00, 0x08, 0x14, 0x22, 0x41,  // <
    0x14, 0x14, 0x14, 0x14, 0x14,  // =
    0x41, 0x22, 0x14, 0x08, 0x00,  // >
    0x02, 0x01, 0x51, 0x09, 0x06,  // ?
    0x32, 0x49, 0x79, 0x41, 0x3E,  // @
    0x7E, 0x11, 0x11, 0x11, 0x7E,  // A
    0x7F, 0x49, 0x49, 0x49, 0x36,  // B
    0x3E, 0x41, 0x41, 0x41, 0x22,  // C
    0x7F, 0x41, 0x41, 0x22, 0x1C,  // D
    0x7F, 0x49, 0x49, 0x49, 0x41,  // E
    0x7F, 0x09, 0x09, 0x01, 0x01,  // F
    0x3E, 0x41, 0x41, 0x51, 0x32,  // G
    0x7F, 0x08, 0x08, 0x08, 0x7F,  // H
    0x00, 0x41, 0x7F, 0x41, 0x00,  // I
    0x20, 0x40, 0x41, 0x3F, 0x01,  // J
    0x7F, 0x08, 0x14, 0x22, 0x41,  // K
    0x7F, 0x40, 0x40, 0x40, 0x40,  // L
    0x7F, 0x02, 0x04, 0x02, 0x7F,  // M
    0x7F, 0x04, 0x08, 0x10, 0x7F,  // N
    0x3E, 0x41, 0x41, 0x41, 0x3E,  // O
    0x7F, 0x09, 0x09, 0x09, 0x06,  // P
    0x3E, 0x41, 0x51, 0x21, 0x5E,  // Q
    0x7F, 0x09, 0x19, 0x29, 0x46,  // R
    0x46, 0x49, 0x49, 0x49, 0x31,  // S
    0x01, 0x01, 0x7F, 0x01, 0x01,  // T
    0x3F, 0x40, 0x40, 0x40, 0x3F,  // U
    0x1F, 0x20, 0x40, 0x20, 0x1F,  // V
    0x7F, 0x20, 0x18, 0x20, 0x7F,  // W
    0x63, 0x14, 0x08, 0x14, 0x63,  // X
    0x03, 0x04, 0x78, 0x04, 0x03,  // Y
    0x61, 0x51, 0x49, 0x45, 0x43,  // Z
    0x00, 0x7F, 0x41, 0x41, 0x00,  // [
    0x02, 0x04, 0x08, 0x10, 0x20,  // backslash
    0x00, 0x41, 0x41, 0x7F, 0x00,  // ]
    0x04, 0x02, 0x01, 0x02, 0x04,  // ^
    0x40, 0x40, 0x40, 0x40, 0x40,  // _
    0x00, 0x01, 0x02, 0x04, 0x00,  // `
    0x20, 0x54, 0x54, 0x54, 0x78,  // a
    0x7F, 0x48, 0x44, 0x44, 0x38,  // b
    0x38, 0x44, 0x44, 0x44, 0x20,  // c
    0x38, 0x44, 0x44, 0x48, 0x7F,  // d
    0x38, 0x54, 0x54, 0x54, 0x18,  // e
    0x08, 0x7E, 0x09, 0x01, 0x02,  // f
    0x08, 0x14, 0x54, 0x54, 0x3C,  // g
    0x7F, 0x08, 0x04, 0x04, 0x78,  // h
    0x00, 0x44, 0x7D, 0x40, 0x00,  // i
    0x20, 0x40, 0x44, 0x3D, 0x00,  // j
    0x00, 0x7F, 0x10, 0x28, 0x44,  // k
    0x00, 0x41, 0x7F, 0x40, 0x00,  // l
    0x7C, 0x04, 0x18, 0x04, 0x78,  // m
    0x7C, 0x08, 0x04, 0x04, 0x78,  // n
    0x38, 0x44, 0x44, 0x44, 0x38,  // o
    0x7C, 0x14, 0x14, 0x14, 0x08,  // p
    0x08, 0x14, 0x14, 0x18, 0x7C,  // q
    0x7C, 0x08, 0x04, 0x04, 0x08,  // r
    0x48, 0x54, 0x54, 0x54, 0x20,  // s
    0x04, 0x3F, 0x44, 0x40, 0x20,  // t
    0x3C, 0x40, 0x40, 0x20, 0x7C,  // u
    0x1C, 0x20, 0x40, 0x20, 0x1C,  // v
    0x3C, 0x40, 0x30, 0x40, 0x3C,  // w
    0x44, 0x28, 0x10, 0x28, 0x44,  // x
    0x0C, 0x50, 0x50, 0x50, 0x3C,  // y
    0x44, 0x64, 0x54, 0x4C, 0x44,  // z
    0x00, 0x08, 0x36, 0x41, 0x00,  // {
    0x00, 0x00, 0x7F, 0x00, 0x00,  // |
    0x00, 0x41, 0x36, 0x08, 0x00,  // }
    0x08, 0x08, 0x2A, 0x1C, 0x08,  // ->
    0x08, 0x1C, 0x2A, 0x08, 0x08   // <-
};

size_t Adafruit_GFX::print(const std::string &str) {
    size_t count = 0;
    // Use the font pointer directly
    const uint8_t* activeFont = font ? font : builtin_font;
    for (char c : str) {
        // Skip non-printable characters
        if (c < ' ' || c > '~') continue;

        // Calculate character position
        int16_t minx = cursor_x;
        int16_t miny = cursor_y;
        int16_t maxx = cursor_x + (fontWidth + 1) * textsize - 1;
        int16_t maxy = cursor_y + fontHeight * textsize - 1;

        // Draw character background
        fillRect(minx, miny, maxx - minx + 1, maxy - miny + 1, textbgcolor);

        // Draw character
        const int char_index = (c - ' ') * fontWidth;
        for (uint8_t col = 0; col < fontWidth; col++) {
            uint8_t font_col = activeFont[char_index + col];
            for (uint8_t row = 0; row < fontHeight; row++) {
                if (font_col & (1 << row)) {
                    fillRect(cursor_x + col * textsize,
                             cursor_y + row * textsize,
                             textsize, textsize, textcolor);
                }
            }
        }

        cursor_x += (fontWidth + 1) * textsize;
        count++;
    }
    return count;
}

size_t Adafruit_GFX::println(const std::string &str) {
    size_t count = print(str); // Draw the string using existing print logic

    // Move to the beginning of the next line
    cursor_x = 0;
    cursor_y += 8 * textsize;

    return count;
}

void Adafruit_GFX::setFont(const uint8_t *f) {
    if (!f) {
        font = builtin_font;
        fontWidth = 5;
        fontHeight = 7;
    } else if (f == profont10_font) {
        font = profont10_font;
        fontWidth = 4;
        fontHeight = 6;
    } else {
        font = f;
        // Fallback: assume 5x7 if unknown
        fontWidth = 5;
        fontHeight = 7;
    }
    // Safety: never allow zero or negative
    if (fontWidth <= 0) fontWidth = 4;
    if (fontHeight <= 0) fontHeight = 6;
}

// Monochrome bitmap (1-bit per pixel)
void Adafruit_GFX::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, 
                             int16_t w, int16_t h, uint16_t color) {
    startWrite();
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            if (pgm_read_byte(bitmap + j * ((w + 7) / 8) + (i / 8)) & (128 >> (i & 7))) {
                writePixel(x + i, y + j, color);
            }
        }
    }
    endWrite();
}

void Adafruit_GFX::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, 
                             int16_t w, int16_t h, uint16_t color, uint16_t bg) {
    startWrite();
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            bool pixel = pgm_read_byte(bitmap + j * ((w + 7) / 8) + (i / 8)) & (128 >> (i & 7));
            writePixel(x + i, y + j, pixel ? color : bg);
        }
    }
    endWrite();
}

// Grayscale bitmap (4-bit per pixel)
void Adafruit_GFX::drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t *bitmap, 
                                      int16_t w, int16_t h) {
    startWrite();
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            // Get 4-bit pixel value (0-15)
            uint8_t pixel = pgm_read_byte(bitmap + j * w + i);
            writePixel(x + i, y + j, pixel);
        }
    }
    endWrite();
}

void Adafruit_GFX::drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t *bitmap, 
                                      int16_t w, int16_t h, uint8_t contrast) {
    startWrite();
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            // Apply contrast to 4-bit pixel
            uint8_t pixel = pgm_read_byte(bitmap + j * w + i);
            pixel = (pixel * contrast) >> 4;  // Scale to 0-15
            writePixel(x + i, y + j, pixel);
        }
    }
    endWrite();
}