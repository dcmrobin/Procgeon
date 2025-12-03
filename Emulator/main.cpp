
#ifdef main
#undef main
#endif

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
    
    // Create a resizable window so we can scale to fullscreen while
    // preserving a square aspect ratio for the game's 128x128 buffer.
    SDL_Window* window = SDL_CreateWindow("Velho",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH * 4, HEIGHT * 4, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        
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

    // viewport / scaling state
    int win_w = WIDTH * 4, win_h = HEIGHT * 4;
    int scale = 4; // pixels per game-pixel
    int offset_x = 0, offset_y = 0; // center offsets in window

    auto recompute_viewport = [&](int ww, int wh) {
        // compute integer scale that fits in window while preserving square
        int s = std::min(ww / WIDTH, wh / HEIGHT);
        if (s < 1) s = 1;
        scale = s;
        int vp_w = WIDTH * scale;
        int vp_h = HEIGHT * scale;
        offset_x = (ww - vp_w) / 2;
        offset_y = (wh - vp_h) / 2;
    };

    // initialize viewport
    SDL_GetWindowSize(window, &win_w, &win_h);
    recompute_viewport(win_w, win_h);
    
    while (running) {
        // Handle input
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    win_w = event.window.data1;
                    win_h = event.window.data2;
                    recompute_viewport(win_w, win_h);
                }
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_F11) {
                    // Toggle fullscreen desktop
                    Uint32 flags = SDL_GetWindowFlags(window);
                    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                        SDL_SetWindowFullscreen(window, 0);
                    } else {
                        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    }
                    // recompute for current window size after mode change
                    SDL_GetWindowSize(window, &win_w, &win_h);
                    recompute_viewport(win_w, win_h);
                }
            }
        }

        // Run game logic
        game_loop();

        // Render to SDL
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render SSD1327 pixel buffer scaled to centered square viewport
        uint8_t* buffer = display.getBuffer();
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                uint8_t level = buffer[y * WIDTH + x];
                uint8_t brightness = level * 17; // Scale to 0-255
                SDL_SetRenderDrawColor(renderer,
                    brightness, brightness, brightness, 255);

                SDL_Rect rect = { offset_x + x * scale, offset_y + y * scale, scale, scale };
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