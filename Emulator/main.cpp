/*#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <cstdint>

const int WIDTH = 128;
const int HEIGHT = 128;

uint8_t displayBuffer[HEIGHT][WIDTH]; // 0–15 grayscale

void drawTestPattern() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            displayBuffer[y][x] = (x + y) % 16; // checkered grayscale
        }
    }
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("SSD1327 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH * 4, HEIGHT * 4, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    drawTestPattern();

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                uint8_t level = displayBuffer[y][x]; // 0–15
                uint8_t brightness = level * 17;     // scale to 0–255
                SDL_SetRenderDrawColor(renderer, brightness, brightness, brightness, 255);
                SDL_Rect pixel = { x * 4, y * 4, 4, 4 }; // 4x zoom
                SDL_RenderFillRect(renderer, &pixel);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60fps
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}*/

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "Adafruit_SSD1327_emu.h"
#include "game.h"

const int WIDTH = 128;
const int HEIGHT = 128;

Adafruit_SSD1327 display; // Global display object

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("SSD1327 Emulator", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        WIDTH * 4, HEIGHT * 4, SDL_WINDOW_SHOWN);
        
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED);

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

        SDL_RenderPresent(renderer);
        SDL_Delay(33); // ~30 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
