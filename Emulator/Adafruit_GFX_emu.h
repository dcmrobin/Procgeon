#pragma once
#include <cstdint>
#include <string>
#include <algorithm>

class Adafruit_GFX {
public:
    int fontWidth;
    int fontHeight;
    int16_t _width, _height;
    Adafruit_GFX(int16_t w, int16_t h);
    virtual ~Adafruit_GFX() = default;
    
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    
    // Drawing functions
    void startWrite();
    void endWrite();
    
    void writePixel(int16_t x, int16_t y, uint16_t color);
    void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    
    void fillScreen(uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color);
    void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t radius, uint16_t color);
    
    void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
    void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
    void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);
    
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
    
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    
    // Text functions
    void setTextSize(uint8_t s);
    void setTextColor(uint16_t c);
    void setTextColor(uint16_t c, uint16_t b);
    void setTextWrap(bool w);
    void setCursor(int16_t x, int16_t y);

    // Bitmap drawing functions
    void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, 
                   int16_t w, int16_t h, uint16_t color);
    
    void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, 
                   int16_t w, int16_t h, uint16_t color, uint16_t bg);
    
    void drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t *bitmap, 
                            int16_t w, int16_t h);
    
    void drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t *bitmap, 
                            int16_t w, int16_t h, uint8_t contrast);
    
    size_t print(const std::string &str);
    size_t print(char c);
    size_t println(const std::string &str);
    
    // Font support
    void setFont(const uint8_t *f = nullptr);
    static const uint8_t profont10_font[];
    // Simple built-in font (5x7)
    static const uint8_t builtin_font[];
    
protected:
    int16_t cursor_x = 0, cursor_y = 0;
    uint16_t textcolor = 15, textbgcolor = 0;
    uint8_t textsize = 1;
    bool wrap = true;
    const uint8_t *font = nullptr;
    
    void charBounds(char c, int16_t *x, int16_t *y, int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy);
};