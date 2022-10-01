#ifndef _TEXT_H_
#define _TEXT_H_

#include <SDL2/SDL_ttf.h>
#include <stdint.h>

typedef struct {
    uint32_t x, y;
    uint16_t w, h;
    char* str;
} Text;

TTF_Font* Text_LoadFont( // written for fast font loading, the path argument can be NULL
    const char* path,
    const int size
);

void Text_Render(
    SDL_Renderer* renderer,
    const Text text,
    TTF_Font* font,
    const SDL_Color color,
    const SDL_bool adjust_size
);

#endif