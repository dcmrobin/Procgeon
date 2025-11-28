#ifndef SDL_WRAPPER_H
#define SDL_WRAPPER_H

// Isolate SDL completely
#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif

#ifdef main
#undef main
#endif

// Fix Windows API conflicts
#ifdef WIN32
#undef WIN32
#endif

// Include SDL with minimal dependencies
#include <SDL.h>

#endif // SDL_WRAPPER_H