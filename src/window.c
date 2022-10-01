#include "window.h"

#include <SDL2/SDL.h>

int Window_Init(SDL_Window** window, SDL_Renderer** renderer, const char* title, const int win_w, const int win_h)
{
    if (SDL_WasInit(SDL_INIT_VIDEO) && SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "ERROR: Failed to init SDL. %s\n", SDL_GetError());
        return -1;
    };

    *window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        win_w,
        win_h,
        0
    );

    if (!*window) {
        fprintf(stderr, "ERROR: Failed to create window. %s\n", SDL_GetError());
        SDL_Quit(); return -1;
    }

    *renderer = SDL_CreateRenderer(
        *window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (!*renderer) {
        fprintf(stderr, "ERROR: Failed to create renderer. %s\n", SDL_GetError());
        SDL_DestroyWindow(*window); SDL_Quit(); return -1;
    }

    return 0;
}

void Window_Quit(SDL_Window* window, SDL_Renderer* renderer)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}