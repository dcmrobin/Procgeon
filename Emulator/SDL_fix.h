#ifndef SDL_FIX_H
#define SDL_FIX_H

#define SDL_MAIN_HANDLED

#ifdef main
#undef main
#endif

// Include SDL headers with correct paths
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#endif // SDL_FIX_H