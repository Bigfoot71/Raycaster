#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_render.h>

int Window_Init(
    SDL_Window** window,
    SDL_Renderer** renderer, 
    const char* title,
    const int win_w,
    const int win_h
);

void Window_Quit(
    SDL_Window* window,
    SDL_Renderer* renderer
);

#endif