#ifndef TRANSLATION_H
#define TRANSLATION_H

#ifdef main
#undef main
#endif

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
#include <cmath>

// SDL includes
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

// --- Arduino macros / stubs ---
#define F(x) x
#define u8g2_font_profont10_mf nullptr
#define u8g2_font_profont12_tr nullptr
#define INPUT_PULLUP 0

// --- Arduino constants ---
#define OUTPUT 0
#define INPUT 1  
#define HIGH 1
#define LOW 0
#define BUILTIN_SDCARD 0 // Dummy value

// --- File mode constants ---
#ifndef FILE_WRITE
#define FILE_WRITE "wb"
#endif

#ifndef FILE_READ  
#define FILE_READ "rb"
#endif

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

// Random functions
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
        size_t write(const void* buffer, size_t len) {
            if (!fp) return 0;
            return std::fwrite(buffer, 1, len, fp);
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
    
    // SIMPLIFY: Remove one of the overloaded open functions
    File open(const std::string& path, const char* mode = "rb") { 
        return File(path, mode); 
    }
    // Remove this duplicate: File open(const std::string& path) { return File(path, "rb"); }
};

// global instance so you can call SD.begin(), SD.exists(), SD.open(), etc.
inline SDClass SD;

// global instance so you can call SD.begin(), SD.exists(), SD.open(), etc.
inline SDClass SD;

// --- Serial (prints to stdout) ---
class SerialClass {
private:
    std::vector<char> inputBuffer;
public:
    void begin(int) {}
    
    // Add these missing methods:
    int available() { 
        // For simulation, always return 0 (no serial input)
        return 0; 
    }
    
    char read() {
        if (inputBuffer.empty()) return -1;
        char c = inputBuffer.back();
        inputBuffer.pop_back();
        return c;
    }
    
    void write(char c) {
        // For simulation, just print
        std::cout << c;
    }
    
    void flush() {
        // Clear any buffered data
        inputBuffer.clear();
        std::cout << std::flush;
    }
    // End of new methods

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
    
    // Add operator! for compatibility
    bool operator!() const { return false; } // Serial is always "available" in emulation
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

// --- Swap / Max / Min ---
template<typename T> inline void swap(T& a, T& b) { T tmp = a; a = b; b = tmp; }
template<typename T> inline T max(T a, T b) { return (a > b) ? a : b; }
template<typename T> inline T min(T a, T b) { return (a < b) ? a : b; }

// --- Audio System Stubs for SDL2 ---
class AudioPlayQueue {
private:
    bool bufferReady = true;
public:
    bool available() { return bufferReady; }
    int16_t* getBuffer() { 
        static int16_t buffer[AUDIO_BLOCK_SAMPLES]; 
        bufferReady = false;
        return buffer; 
    }
    void playBuffer() { 
        bufferReady = true; 
    }
};

class AudioMixer4 {
public:
    void gain(int channel, float volume) {
        // Store volume per channel for SDL2 implementation
    }
};

class AudioOutputI2S {
    // Empty stub - SDL2 handles output
};

class AudioControlSGTL5000 {
public:
    void enable() {}
    void volume(float volume) {
        // Set master volume in SDL2
        Mix_Volume(-1, static_cast<int>(volume * MIX_MAX_VOLUME));
    }
};

// Stub AudioConnection (does nothing in SDL2)
class AudioConnection {
public:
    template<typename... Args>
    AudioConnection(Args&&...) {} // Accept any parameters
};

// Stub AudioMemory
inline void AudioMemory(int) {}

// Audio interrupt stubs (for SaveLogic compatibility)
inline void AudioNoInterrupts() {}
inline void AudioInterrupts() {}
class Adafruit_SSD1327;
// U8G2 emulation
class U8G2_FOR_ADAFRUIT_GFX {
public:
    void setFont(const uint8_t* font) {}
    void setCursor(int x, int y) {}
    void print(const char* text) {}
    void println(const char* text) {}
    
    // Add missing methods for compatibility
    void begin(Adafruit_SSD1327& display) {} // Dummy implementation
    void setForegroundColor(uint16_t color) {} // Dummy implementation
    int getUTF8Width(const char* text) { 
        // Simple approximation - 6 pixels per character
        return strlen(text) * 6; 
    }
};
extern U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

// --- Audio (SDL_mixer-backed stubs) ---
constexpr int MAX_SIMULTANEOUS_SFX = 8;
constexpr int NUM_SFX = 25;

class AudioPlaySdWav {
private:
    Mix_Chunk* chunk = nullptr;
    int channel = -1;
    static inline std::unordered_map<std::string, Mix_Chunk*> loadedChunks;
    float currentVolume = 1.0f;
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
        if (channel != -1) {
            Mix_Volume(channel, static_cast<int>(currentVolume * MIX_MAX_VOLUME));
        }
        return channel != -1;
    }
    void play() { 
        if (chunk) {
            channel = Mix_PlayChannel(-1, chunk, 0); 
            if (channel != -1) {
                Mix_Volume(channel, static_cast<int>(currentVolume * MIX_MAX_VOLUME));
            }
        }
    }
    void stop() { if (channel != -1) Mix_HaltChannel(channel); channel = -1; }
    bool isPlaying() { return channel != -1 && Mix_Playing(channel) != 0; }
    void volume(float level) {
        currentVolume = constrain(level, 0.0f, 1.0f);
        if (chunk) {
            Mix_VolumeChunk(chunk, static_cast<int>(currentVolume * MIX_MAX_VOLUME));
        }
        if (channel != -1) {
            Mix_Volume(channel, static_cast<int>(currentVolume * MIX_MAX_VOLUME));
        }
    }
    
    void setVolume(float level) {
        volume(level);
    }
};
inline AudioPlaySdWav playWav1;
inline AudioPlaySdWav playWav2;

// SDL2 helper wrappers
inline bool initSDL2Audio(int freq = 44100, Uint16 format = MIX_DEFAULT_FORMAT, int channels = 2, int chunksize = 1024) {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        std::printf("SDL audio init failed: %s\n", SDL_GetError());
        return false;
    }
    if (Mix_OpenAudio(freq, format, channels, chunksize) < 0) {
        std::printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
        return false;
    }
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
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

inline Mix_Chunk* loadWav(const std::string& path) { 
    Mix_Chunk* c = Mix_LoadWAV(path.c_str()); 
    if (!c) std::printf("Failed to load WAV: %s\n", path.c_str()); 
    return c; 
}

// Raw SFX playback structure for SDL2 compatibility
struct RawSFXPlayback {
    const int16_t* data = nullptr;
    size_t samplesTotal = 0;
    size_t samplesPlayed = 0;
    bool isPlaying = false;
    float volume = 1.0f;
};

// Forward declarations for GameAudio compatibility
extern int ambientNoiseLevel;
extern int masterVolume;
extern float jukeboxVolume;
void setJukeboxVolume(float v);
bool playRawSFX(int sfxIndex);
bool playRawSFX3D(int sfxIndex, float soundX, float soundY);
void serviceRawSFX();
void initAudio();
void freeSFX();
bool loadSFXtoRAM();

// end include guard
#endif // TRANSLATION_H