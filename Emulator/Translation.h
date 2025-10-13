#ifndef TRANSLATION_H
#define TRANSLATION_H

#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <SDL2/SDL_ttf.h>
#include <map>

class StringImpl {
public:
    std::string value;

    // --- Constructors ---
    StringImpl() = default;
    StringImpl(const char* s) : value(s ? s : "") {}
    StringImpl(const std::string& s) : value(s) {}
    StringImpl(char c) : value(1, c) {}
    StringImpl(int v) : value(std::to_string(v)) {}
    StringImpl(unsigned int v) : value(std::to_string(v)) {}
    StringImpl(long v) : value(std::to_string(v)) {}
    StringImpl(unsigned long v) : value(std::to_string(v)) {}
    StringImpl(float v) { std::ostringstream oss; oss << v; value = oss.str(); }
    StringImpl(double v) { std::ostringstream oss; oss << v; value = oss.str(); }

    // --- Assignment ---
    StringImpl& operator=(const char* s) { value = s ? s : ""; return *this; }
    StringImpl& operator=(const std::string& s) { value = s; return *this; }
    StringImpl& operator=(const StringImpl& other) { value = other.value; return *this; }

    // --- Concatenation (member: String + X) ---
    StringImpl operator+(const StringImpl& other) const { return StringImpl(value + other.value); }
    StringImpl operator+(const char* s) const { return StringImpl(value + (s ? s : "")); }
    StringImpl operator+(char c) const { std::string tmp = value; tmp.push_back(c); return StringImpl(tmp); }
    StringImpl operator+(int v) const { return StringImpl(value + std::to_string(v)); }
    StringImpl operator+(float v) const { std::ostringstream oss; oss << v; return StringImpl(value + oss.str()); }
    StringImpl operator+(double v) const { std::ostringstream oss; oss << v; return StringImpl(value + oss.str()); }

    StringImpl& operator+=(const StringImpl& other) { value += other.value; return *this; }
    StringImpl& operator+=(const char* s) { value += (s ? s : ""); return *this; }
    StringImpl& operator+=(int v) { value += std::to_string(v); return *this; }

    // --- Comparison ---
    bool operator==(const StringImpl& other) const { return value == other.value; }
    bool operator!=(const StringImpl& other) const { return value != other.value; }

    // Arduino-style equals()
    bool equals(const StringImpl& other) const { return value == other.value; }
    bool equals(const char* s) const { return value == (s ? s : ""); }

    // --- Accessors similar to Arduino ---
    const char* c_str() const { return value.c_str(); }
    int length() const { return static_cast<int>(value.length()); }
    bool isEmpty() const { return value.empty(); }

    // charAt and setCharAt like Arduino
    char charAt(int index) const {
        if (index < 0 || index >= static_cast<int>(value.size())) return '\0';
        return value[static_cast<size_t>(index)];
    }
    void setCharAt(int index, char c) {
        if (index < 0) return;
        if (index >= static_cast<int>(value.size())) {
            // If index beyond end, we can choose to expand with '\0' or ignore.
            // Arduino's behavior: sets if in range; we'll ignore out-of-range.
            return;
        }
        value[static_cast<size_t>(index)] = c;
    }

    // substring (Arduino-style: substring(from), substring(from, to))
    StringImpl substring(int from) const {
        if (from < 0) from = 0;
        if (from >= static_cast<int>(value.size())) return StringImpl("");
        return StringImpl(value.substr(static_cast<size_t>(from)));
    }
    StringImpl substring(int from, int to) const {
        if (from < 0) from = 0;
        if (to <= from) return StringImpl("");
        if (from >= static_cast<int>(value.size())) return StringImpl("");
        int safeTo = std::min<int>(to, static_cast<int>(value.size()));
        return StringImpl(value.substr(static_cast<size_t>(from), static_cast<size_t>(safeTo - from)));
    }

    // numeric conversions
    long toInt() const {
        try { return std::stol(value); } catch (...) { return 0; }
    }
    float toFloat() const {
        try { return std::stof(value); } catch (...) { return 0.0f; }
    }

    // explicit conversion to std::string
    operator std::string() const { return value; }
};

// Make plain symbol String identical to StringImpl so existing code can use "String"
#define String StringImpl

// Free operators to support LHS const char* and std::string
inline String operator+(const char* lhs, const String& rhs) {
    return String(lhs ? lhs : "") + rhs;
}
inline String operator+(const std::string& lhs, const String& rhs) {
    return String(lhs) + rhs;
}
inline String operator+(const char* lhs, int rhs) {
    return String(lhs ? lhs : "") + String(rhs);
}
inline String operator+(const char* lhs, float rhs) {
    return String(lhs ? lhs : "") + String(rhs);
}

class U8G2_FOR_ADAFRUIT_GFX {
public:
    U8G2_FOR_ADAFRUIT_GFX() : cursorX(0), cursorY(0), renderer(nullptr), currentFont(nullptr) {}

    void begin() {
        // nothing here; SDL and TTF should be initialized outside
    }

    // Set the SDL renderer to draw to
    void setRenderer(SDL_Renderer* ren) { renderer = ren; }

    // Load font from TTF file path (maps roughly to U8g2 font handle)
    bool setFont(const char* fontPath, int fontSize = 16) {
        if (!renderer) return false;
        TTF_Font* fnt = TTF_OpenFont(fontPath, fontSize);
        if (!fnt) return false;
        currentFont = fnt;
        return true;
    }

    // Set cursor position
    void setCursor(int x, int y) { cursorX = x; cursorY = y; }

    // Draw string at cursor position (or x,y if given)
    void drawStr(int x, int y, const char* str) {
        if (!renderer || !currentFont || !str) return;
        SDL_Color color = {255, 255, 255, 255}; // white color
        SDL_Surface* surface = TTF_RenderText_Solid(currentFont, str, color);
        if (!surface) return;
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect dstRect = { x, y, surface->w, surface->h };
        SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }

    void drawStr(const char* str) { drawStr(cursorX, cursorY, str); }

    void clearBuffer() { /* nothing: SDL renders immediately */ }
    void sendBuffer() { /* nothing: SDL renders immediately */ }

    void setCursorColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a=255) {
        textColor = {r,g,b,a};
    }

private:
    int cursorX, cursorY;
    SDL_Renderer* renderer;
    TTF_Font* currentFont;
    SDL_Color textColor;
};

template <typename T>
inline void swap(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}
template <typename T>
inline T max(T a, T b) {
    return (a > b) ? a : b;
}
int random(int min, int max);
int random(int max);
float constrain(float val, float min, float max);

#endif