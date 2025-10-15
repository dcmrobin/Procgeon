#ifndef TRANSLATION_H
#define TRANSLATION_H

// --- Arduino compatibility shims for SDL2 build ---
#ifndef ARDUINO
    // Define PROGMEM as empty for desktop builds
    #ifndef PROGMEM
    #define PROGMEM
    #endif

    // Emulate pgm_read_byte and friends
    #ifndef pgm_read_byte
    #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
    #endif
    #ifndef pgm_read_word
    #define pgm_read_word(addr) (*(const uint16_t *)(addr))
    #endif
    #ifndef pgm_read_dword
    #define pgm_read_dword(addr) (*(const uint32_t *)(addr))
    #endif
#endif

/*#ifndef ARDUINO
    #include <cstdlib>
    #include <algorithm>

    // Random replacements
    inline long random(long min, long max) {
        return min + (std::rand() % (max - min));
    }
    inline float random(float min, float max) {
        float scale = std::rand() / (float)RAND_MAX;
        return min + scale * (max - min);
    }

    // Constrain replacement
    template<typename T>
    inline T constrain(T val, T low, T high) {
        return std::max(low, std::min(high, val));
    }
#endif*/
// --- end Arduino compatibility section ---

// Single-header Arduino/Teensy -> SDL2 translation shim.
// Provides: String, Serial, millis/random, SD (File), EEPROM, Audio stubs (SDL_mixer),
// U8G2 minimal SDL renderer using SDL_ttf, pinMode/digital/analog/delay, etc.

#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <algorithm>
#include <map>
#include <chrono>
#include <unordered_map>
#include <filesystem>
#include <cstdio>
#include <cstdint>
#include <type_traits>

// SDL includes
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

// --- Arduino macros / stubs ---
#define F(x) x
#define u8g2_font_profont10_mf nullptr
#define u8g2_font_profont12_tr nullptr
#define INPUT_PULLUP 0

// --- Arduino analog/digital stubs ---
#ifndef A0
#define A0 0
#endif

inline int analogRead(int) {
    // simulate some noise (10-bit)
    return std::rand() % 1024;
}
inline int digitalRead(int) {
    // default: not pressed (HIGH)
    return 1;
}
inline void pinMode(int, int) {}
inline void delay(unsigned long ms) { SDL_Delay(static_cast<Uint32>(ms)); }

constexpr size_t AUDIO_BLOCK_SAMPLES = 128;

// --- Time, random, constrain ---
inline auto millis_start = std::chrono::steady_clock::now();
inline unsigned long millis() {
    auto now = std::chrono::steady_clock::now();
    return static_cast<unsigned long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - millis_start).count()
    );
}

// Fixed: Remove inline to avoid multiple definition issues
/*int random(int min, int max);
int random(int max);
void randomSeed(unsigned long seed);
float constrain(float val, float min, float max);*/

// Implementations (keep inline for header-only use)
inline int random(int min, int max) { 
    return (max > min) ? min + std::rand() % (max - min) : min; 
}

inline int random(int max) { 
    return (max > 0) ? std::rand() % max : 0; 
}

inline void randomSeed(unsigned long seed) { 
    std::srand(static_cast<unsigned int>(seed)); 
}

inline float constrain(float val, float min, float max) { 
    if (val < min) return min; 
    if (val > max) return max; 
    return val; 
}

// Template version for other types
template<typename T>
inline T constrain(T val, T min, T max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

// --- SD emulation ---
constexpr int SD_CS = 0; // dummy for SDL2

class SDClass {
public:
    class File {
    private:
        std::FILE* fp = nullptr;
    public:
        File() = default;
        File(const std::string& path, const char* mode = "rb") {
            fp = std::fopen(path.c_str(), mode);
        }
        bool available() const { return fp != nullptr; }
        size_t size() const {
            if (!fp) return 0;
            auto pos = std::ftell(fp);
            std::fseek(fp, 0, SEEK_END);
            size_t len = std::ftell(fp);
            std::fseek(fp, pos, SEEK_SET);
            return len;
        }
        size_t read(void* buffer, size_t len) {
            if (!fp) return 0;
            return std::fread(buffer, 1, len, fp);
        }
        void close() {
            if (fp) {
                std::fclose(fp);
                fp = nullptr;
            }
        }
        operator bool() const { return fp != nullptr; }
    };

    bool begin(int) { return true; }
    bool exists(const std::string& path) { return std::filesystem::exists(path); }
    File open(const std::string& path) { return File(path, "rb"); }
};

// global instance so you can call SD.begin(), SD.exists(), SD.open(), etc.
inline SDClass SD;

// --- Serial (prints to stdout) ---
class SerialClass {
public:
    void begin(int) {}
    template<typename T> void print(const T& val) { std::cout << val; }
    template<typename T> void println(const T& val) { std::cout << val << std::endl; }
    void println() { std::cout << std::endl; }

    // printf-like convenience
    template<typename... Args>
    void printf(const char* fmt, Args... args) {
        char buf[1024];
        std::snprintf(buf, sizeof(buf), fmt, args...);
        std::cout << buf;
    }
};
inline SerialClass Serial;

// --- EEPROM (simple in-memory storage) ---
struct EEPROMClass {
    std::unordered_map<int,int> mem;
    int read(int addr) { return mem.count(addr) ? mem[addr] : 0; }
    void write(int addr, int val) { mem[addr] = val; }
    void update(int addr, int val) { if (read(addr) != val) write(addr, val); }
};
inline EEPROMClass EEPROM;

// --- StringImpl (Arduino-like String wrapper) ---
class StringImpl {
public:
    std::string value;

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

    StringImpl& operator=(const char* s) { value = s ? s : ""; return *this; }
    StringImpl& operator=(const std::string& s) { value = s; return *this; }
    StringImpl& operator=(const StringImpl& other) { value = other.value; return *this; }

    StringImpl operator+(const StringImpl& o) const { return StringImpl(value + o.value); }
    StringImpl operator+(const char* s) const { return StringImpl(value + (s ? s : "")); }
    StringImpl operator+(char c) const { std::string tmp = value; tmp.push_back(c); return StringImpl(tmp); }
    StringImpl operator+(int v) const { return StringImpl(value + std::to_string(v)); }
    StringImpl operator+(float v) const { std::ostringstream oss; oss << v; return StringImpl(value + oss.str()); }
    StringImpl operator+(double v) const { std::ostringstream oss; oss << v; return StringImpl(value + oss.str()); }

    StringImpl& operator+=(const StringImpl& o) { value += o.value; return *this; }
    StringImpl& operator+=(const char* s) { value += (s ? s : ""); return *this; }
    StringImpl& operator+=(int v) { value += std::to_string(v); return *this; }

    bool operator==(const StringImpl& o) const { return value == o.value; }
    bool operator!=(const StringImpl& o) const { return value != o.value; }
    bool equals(const StringImpl& o) const { return value == o.value; }
    bool equals(const char* s) const { return value == (s ? s : ""); }

    const char* c_str() const { return value.c_str(); }
    int length() const { return static_cast<int>(value.length()); }
    bool isEmpty() const { return value.empty(); }

    int indexOf(const std::string& substring) const {
        size_t pos = value.find(substring);
        return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
    }
    int indexOf(const char* substring) const {
        size_t pos = value.find(substring);
        return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
    }

    void remove(int index, int count = 1) {
        if (index >= 0 && index < static_cast<int>(value.size())) value.erase(index, count);
    }

    char charAt(int i) const { if (i < 0 || i >= static_cast<int>(value.size())) return '\0'; return value[static_cast<size_t>(i)]; }
    char operator[](int i) const { return charAt(i); }
    void setCharAt(int i, char c) { if (i < 0 || i >= static_cast<int>(value.size())) return; value[static_cast<size_t>(i)] = c; }

    StringImpl substring(int from) const { if (from < 0) from = 0; if (from >= static_cast<int>(value.size())) return StringImpl(""); return StringImpl(value.substr(static_cast<size_t>(from))); }
    StringImpl substring(int from, int to) const { if (from < 0) from = 0; if (to <= from) return StringImpl(""); int safeTo = std::min<int>(to, static_cast<int>(value.size())); return StringImpl(value.substr(static_cast<size_t>(from), static_cast<size_t>(safeTo - from))); }

    long toInt() const { try { return std::stol(value); } catch (...) { return 0; } }
    float toFloat() const { try { return std::stof(value); } catch (...) { return 0.0f; } }
    operator std::string() const { return value; }
};
#define String StringImpl
inline String operator+(const char* lhs, const String& rhs) { return String(lhs ? lhs : "") + rhs; }
inline String operator+(const std::string& lhs, const String& rhs) { return String(lhs) + rhs; }

// --- Swap / Max ---
template<typename T> inline void swap(T& a, T& b) { T tmp = a; a = b; b = tmp; }
template<typename T> inline T max(T a, T b) { return (a > b) ? a : b; }

// --- Audio (SDL_mixer-backed stubs) ---
constexpr int MAX_SIMULTANEOUS_SFX = 4;
constexpr int NUM_SFX = 16;

class AudioPlaySdWav {
private:
    Mix_Chunk* chunk = nullptr;
    int channel = -1;
    static inline std::unordered_map<std::string, Mix_Chunk*> loadedChunks;
public:
    AudioPlaySdWav() = default;
    bool play(const char* filename) {
        if (!filename) return false;
        std::string fname = filename;
        if (loadedChunks.find(fname) == loadedChunks.end()) {
            Mix_Chunk* c = Mix_LoadWAV(fname.c_str());
            if (!c) { std::printf("Failed to load WAV: %s\n", fname.c_str()); return false; }
            loadedChunks[fname] = c;
        }
        chunk = loadedChunks[fname];
        channel = Mix_PlayChannel(-1, chunk, 0);
        return channel != -1;
    }
    void play() { if (chunk) channel = Mix_PlayChannel(-1, chunk, 0); }
    void stop() { if (channel != -1) Mix_HaltChannel(channel); channel = -1; }
    bool isPlaying() { return channel != -1 && Mix_Playing(channel) != 0; }
};

class AudioPlayQueue {
public:
    Mix_Chunk* chunk = nullptr;
    bool available() { return true; }
    int16_t* getBuffer() { return nullptr; } // Teensy provides a buffer — here we don't
    void playBuffer() { if (chunk) Mix_PlayChannel(-1, chunk, 0); }
    void setChunk(Mix_Chunk* c) { chunk = c; }
};

class AudioMixer4 {
public:
    float gainLevel[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    void gain(int ch, float g) { if (ch >= 0 && ch < 4) gainLevel[ch] = g; }
};

class AudioOutputI2S { public: void begin() {} };
inline void AudioMemory(int) {}

class AudioControlSGTL5000 {
public:
    void enable() {}
    void volume(float) {}
};

class AudioConnection {
public:
    template<typename A, typename B>
    AudioConnection(A& a, int aCh, B& b, int bCh) { (void)a; (void)aCh; (void)b; (void)bCh; }
};

inline AudioPlayQueue queue[MAX_SIMULTANEOUS_SFX];
inline AudioMixer4 mixer1;
inline AudioMixer4 musicMixer;
inline AudioOutputI2S audioOutput;
inline AudioPlaySdWav playWav1;
inline AudioControlSGTL5000 sgtl5000_1;

// SDL2 helper wrappers
inline bool initSDL2Audio(int freq = 44100, Uint16 format = MIX_DEFAULT_FORMAT, int channels = 2, int chunksize = 1024) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) return false;
    if (Mix_OpenAudio(freq, format, channels, chunksize) < 0) return false;
    Mix_AllocateChannels(MAX_SIMULTANEOUS_SFX * 2);
    return true;
}

inline bool initSDL2TTF() {
    return TTF_Init() == 0;
}

inline void closeSDL2TTF() {
    TTF_Quit();
}

inline void closeSDL2Audio() {
    Mix_CloseAudio();
}

inline Mix_Chunk* loadWav(const std::string& path) { 
    Mix_Chunk* c = Mix_LoadWAV(path.c_str()); 
    if (!c) std::printf("Failed to load WAV: %s\n", path.c_str()); 
    return c; 
}

// --- U8G2 minimal SDL2-backed implementation (TTF) ---
class U8G2_FOR_ADAFRUIT_GFX {
public:
    U8G2_FOR_ADAFRUIT_GFX() : cursorX(0), cursorY(0), renderer(nullptr), currentFont(nullptr) {}
    ~U8G2_FOR_ADAFRUIT_GFX() {
        if (currentFont) {
            TTF_CloseFont(currentFont);
            currentFont = nullptr;
        }
    }
    
    void begin() {}
    void setForegroundColor(int) { (void)0; }
    template<typename T> void begin(T&) { /* accept Adafruit display parameter and ignore */ }

    void setRenderer(SDL_Renderer* ren) { renderer = ren; }
    bool setFont(const char* fontPath, int fontSize = 12) {
        fontSize = 12;
        fontPath = "Fonts/PixelifySans-VariableFont_wght.ttf";
        if (!renderer) return false;
        if (currentFont) {
            TTF_CloseFont(currentFont);
            currentFont = nullptr;
        }
        TTF_Font* fnt = TTF_OpenFont(fontPath, fontSize);
        if (!fnt) return false;
        currentFont = fnt; 
        return true;
    }

    void setCursor(int x, int y) { cursorX = x; cursorY = y; }

    void drawStr(int x, int y, const char* str) {
        if (!renderer || !currentFont || !str) return;
        SDL_Color color = {255, 255, 255, 255};
        SDL_Surface* s = TTF_RenderUTF8_Solid(currentFont, str, color);
        if (!s) return;
        SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
        SDL_Rect r = { x, y, s->w, s->h };
        SDL_RenderCopy(renderer, t, nullptr, &r);
        SDL_DestroyTexture(t);
        SDL_FreeSurface(s);
    }
    void drawStr(const char* str) { drawStr(cursorX, cursorY, str); }
    void clearBuffer() {}
    void sendBuffer() {}
    void setCursorColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255) { textColor = { r,g,b,a }; }

    template<typename T>
    void print(const T& val) {
        if constexpr (std::is_same_v<T, StringImpl>) {
            drawStr(val.c_str());
        } else if constexpr (std::is_convertible_v<T, std::string>) {
            drawStr(static_cast<std::string>(val).c_str());
        } else if constexpr (std::is_arithmetic_v<T>) {
            drawStr(std::to_string(val).c_str());
        } else {
            drawStr("<?>");
        }
    }
    void print(const char* str) { drawStr(str); }
    void print(const std::string& str) { drawStr(str.c_str()); }

    template<typename T> void println(const T& val) { print(val); cursorY += lineHeight(); cursorX = 0; }
    void println(const char* str) { print(str); cursorY += lineHeight(); cursorX = 0; }
    void println(const std::string& str) { print(str.c_str()); cursorY += lineHeight(); cursorX = 0; }

    int getUTF8Width(const char* text) {
        if (!currentFont || !text) return 0;
        int w = 0, h = 0;
        TTF_SizeUTF8(currentFont, text, &w, &h);
        return w;
    }

private:
    int cursorX;
    int cursorY;
    SDL_Renderer* renderer;
    TTF_Font* currentFont;
    SDL_Color textColor;

    int lineHeight() const {
        if (!currentFont) return 10;
        return TTF_FontHeight(currentFont);
    }
};
extern U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;
// end include guard
#endif // TRANSLATION_H