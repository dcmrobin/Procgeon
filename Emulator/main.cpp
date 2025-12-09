#ifdef main
#undef main
#endif

#include "Adafruit_SSD1327_emu.h"
#include "game.h"
#include "Translation.h"
#include "GameAudio.h"
#include <algorithm>

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
    
    // Create a resizable window that starts in windowed mode
    SDL_Window* window = SDL_CreateWindow("Velho",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH * 4, HEIGHT * 4, 
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP);
        
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
    int win_w, win_h;
    int scale = 1; // pixels per game-pixel
    int offset_x = 0, offset_y = 0; // center offsets in window
    
    // Track windowed mode size for restoring
    int windowed_width = WIDTH * 4;
    int windowed_height = HEIGHT * 4;

    auto recompute_viewport = [&](int ww, int wh) {
        // compute integer scale that fits in window while preserving square aspect ratio
        int s = std::min(ww / WIDTH, wh / HEIGHT);
        if (s < 1) s = 1;  // Minimum scale of 1
        scale = s;
        int vp_w = WIDTH * scale;
        int vp_h = HEIGHT * scale;
        offset_x = (ww - vp_w) / 2;
        offset_y = (wh - vp_h) / 2;
        
        // Optional: print debug info
        printf("Window: %dx%d, Scale: %d, Viewport: %dx%d, Offset: %d,%d\n", 
               ww, wh, scale, vp_w, vp_h, offset_x, offset_y);
    };

    // Get initial window size and compute viewport
    SDL_GetWindowSize(window, &win_w, &win_h);
    recompute_viewport(win_w, win_h);
    
    while (running) {
        // Handle input
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                    event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    win_w = event.window.data1;
                    win_h = event.window.data2;
                    recompute_viewport(win_w, win_h);
                    
                    // Save windowed size when not in fullscreen
                    Uint32 flags = SDL_GetWindowFlags(window);
                    if (!(flags & SDL_WINDOW_FULLSCREEN_DESKTOP)) {
                        windowed_width = win_w;
                        windowed_height = win_h;
                    }
                }
                else if (event.window.event == SDL_WINDOWEVENT_MINIMIZED) {
                    // Window minimized - you might want to pause the game here
                }
                else if (event.window.event == SDL_WINDOWEVENT_RESTORED) {
                    // Window restored from minimized
                }
            } else if (event.type == SDL_KEYDOWN) {
                // F11 toggles between fullscreen and windowed
                if (event.key.keysym.sym == SDLK_F11 && event.key.repeat == 0) {
                    Uint32 flags = SDL_GetWindowFlags(window);
                    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                        // Exit fullscreen, restore to windowed
                        SDL_SetWindowFullscreen(window, 0);
                        SDL_SetWindowSize(window, windowed_width, windowed_height);
                        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                        SDL_RestoreWindow(window); // Ensure window is not minimized
                    } else {
                        // Enter fullscreen
                        // Save current window size before going fullscreen
                        SDL_GetWindowSize(window, &windowed_width, &windowed_height);
                        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    }
                    // Get new size and recompute viewport
                    SDL_GetWindowSize(window, &win_w, &win_h);
                    recompute_viewport(win_w, win_h);
                }
                // Alt+Enter also toggles fullscreen (common alternative)
                else if (event.key.keysym.sym == SDLK_RETURN && 
                        (SDL_GetModState() & KMOD_ALT) && event.key.repeat == 0) {
                    Uint32 flags = SDL_GetWindowFlags(window);
                    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                        // Exit fullscreen
                        SDL_SetWindowFullscreen(window, 0);
                        SDL_SetWindowSize(window, windowed_width, windowed_height);
                        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                    } else {
                        // Enter fullscreen
                        SDL_GetWindowSize(window, &windowed_width, &windowed_height);
                        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    }
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
                uint8_t brightness = level * 17; // Scale to 0-255 (0-15 to 0-255)
                SDL_SetRenderDrawColor(renderer,
                    brightness, brightness, brightness, 255);

                SDL_Rect rect = { 
                    offset_x + x * scale, 
                    offset_y + y * scale, 
                    scale, 
                    scale 
                };
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