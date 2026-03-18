#ifdef main
#undef main
#endif

#include "Common.h"
#include "GameState.h"
#include "Adafruit_SSD1327_emu.h"
#include "game.h"
#include "GameAudio.h"

#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// main.cpp  — SDL2 entry point
// Logic identical to original.
// Replaced bare `buttons` references with g_state.buttons.
// ─────────────────────────────────────────────────────────────────────────────

const int WIDTH  = SCREEN_WIDTH;
const int HEIGHT = SCREEN_HEIGHT;

Adafruit_SSD1327 display;

int main() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    if (!initSDL2Audio()) {
        printf("SDL audio initialization failed: %s\n", SDL_GetError());
        // non-fatal — continue without audio
    } else {
        printf("SDL audio initialized successfully\n");
        if (!loadSFXtoRAM())
            printf("Failed to load SFX to RAM\n");
        else
            printf("SFX loaded successfully\n");
    }

    SDL_Window* window = SDL_CreateWindow(
        "Velho",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH * 4, HEIGHT * 4,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP);

    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    display.begin();
    game_setup();

    bool running = true;
    SDL_Event event;

    int   scale    = 1;
    int   offset_x = 0, offset_y = 0;
    int   win_w    = WIDTH * 4, win_h = HEIGHT * 4;
    int   windowed_width  = win_w;
    int   windowed_height = win_h;

    auto recompute_viewport = [&](int ww, int wh) {
        int s = std::min(ww / WIDTH, wh / HEIGHT);
        if (s < 1) s = 1;
        scale    = s;
        offset_x = (ww - WIDTH  * scale) / 2;
        offset_y = (wh - HEIGHT * scale) / 2;
    };

    SDL_GetWindowSize(window, &win_w, &win_h);
    recompute_viewport(win_w, win_h);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                    event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    win_w = event.window.data1;
                    win_h = event.window.data2;
                    recompute_viewport(win_w, win_h);
                    Uint32 flags = SDL_GetWindowFlags(window);
                    if (!(flags & SDL_WINDOW_FULLSCREEN_DESKTOP)) {
                        windowed_width  = win_w;
                        windowed_height = win_h;
                    }
                }
            } else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                bool isF11   = (event.key.keysym.sym == SDLK_F11);
                bool isAltEnt= (event.key.keysym.sym == SDLK_RETURN &&
                                (SDL_GetModState() & KMOD_ALT));
                if (isF11 || isAltEnt) {
                    Uint32 flags = SDL_GetWindowFlags(window);
                    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                        SDL_SetWindowFullscreen(window, 0);
                        SDL_SetWindowSize(window, windowed_width, windowed_height);
                        SDL_SetWindowPosition(window,
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                        SDL_RestoreWindow(window);
                    } else {
                        SDL_GetWindowSize(window, &windowed_width, &windowed_height);
                        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    }
                    SDL_GetWindowSize(window, &win_w, &win_h);
                    recompute_viewport(win_w, win_h);
                }
            }
        }

        game_loop();

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        uint8_t* buffer = display.getBuffer();
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                uint8_t level      = buffer[y * WIDTH + x];
                uint8_t brightness = level * 17;
                SDL_SetRenderDrawColor(renderer, brightness, brightness, brightness, 255);
                SDL_Rect rect = {
                    offset_x + x * scale,
                    offset_y + y * scale,
                    scale, scale
                };
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(3);
    }

    freeSFX();
    closeSDL2Audio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}