#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "Adafruit_SSD1327_emu.h"
#include "game.h"
#include "Translation.h"

const int WIDTH = 128;
const int HEIGHT = 128;

Adafruit_SSD1327 display; // Global display object
U8G2_FOR_ADAFRUIT_GFX u8g2; // Global U8G2 instance

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("SSD1327 Emulator", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        WIDTH * 4, HEIGHT * 4, SDL_WINDOW_SHOWN);
        
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED);

    // Initialize U8G2 with the SDL renderer
    u8g2.setRenderer(renderer);
    
    // Try to load a font - you'll need a .ttf file in your project directory
    if (!u8g2.setFont("arial.ttf", 12)) {
        // Try some common fallback paths
        if (!u8g2.setFont("./fonts/arial.ttf", 12)) {
            if (!u8g2.setFont("C:/Windows/Fonts/arial.ttf", 12)) {
                printf("WARNING: Could not load font for U8G2 text rendering\n");
                printf("Text will not be visible. Please place arial.ttf in your project directory.\n");
            }
        }
    }

    display.begin();
    game_setup();  // Initialize game

    bool running = true;
    SDL_Event event;
    
    while (running) {
        // Handle input
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        // Run game logic
        game_loop();

        // Render to SDL
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render SSD1327 pixel buffer (your original graphics)
        uint8_t* buffer = display.getBuffer();
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                uint8_t level = buffer[y * WIDTH + x];
                uint8_t brightness = level * 17; // Scale to 0-255
                SDL_SetRenderDrawColor(renderer, 
                    brightness, brightness, brightness, 255);
                    
                SDL_Rect rect = {x * 4, y * 4, 4, 4};
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        // NEW: Render U8G2 text on top of the pixel buffer
        // This ensures text appears over your game graphics
        // (U8G2 text rendering happens during game_loop(), but we need to present it here)

        SDL_RenderPresent(renderer);
        SDL_Delay(3); // ~30 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}