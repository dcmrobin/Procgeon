
#ifdef main
#undef main
#endif

#include "SDL_fix.h"
#include "Adafruit_SSD1327_emu.h"
#include "game.h"
#include "Translation.h"
#include "GameAudio.h"

const int WIDTH = 128;
const int HEIGHT = 128;

Adafruit_SSD1327 display; // Global display object

int main() {
    // Initialize SDL with both video and audio
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }
    
    // Initialize SDL audio system
    if (!initSDL2Audio()) {
        printf("SDL audio initialization failed: %s\n", SDL_GetError());
        // Don't return - let it continue without audio
    } else {
        printf("SDL audio initialized successfully\n");
    }

    if (!initSDL2Audio()) {
        printf("SDL audio initialization failed: %s\n", SDL_GetError());
    } else {
        printf("SDL audio initialized successfully\n");
        
        // Load sound effects
        if (!loadSFXtoRAM()) {
            printf("Failed to load SFX to RAM\n");
        } else {
            printf("SFX loaded successfully\n");
        }
    }
    
    SDL_Window* window = SDL_CreateWindow("SSD1327 Emulator", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        WIDTH * 4, HEIGHT * 4, SDL_WINDOW_SHOWN);
        
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
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

        // Render SSD1327 pixel buffer
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
        SDL_Delay(3); // ~30 FPS
    }

    // Cleanup
    freeSFX();
    closeSDL2Audio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}