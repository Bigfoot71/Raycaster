#include "text.h"

#include <SDL2/SDL_ttf.h>

#ifdef __linux__
# define FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
#elif __APPLE__
# define FONT_PATH "/Library/Fonts/Arial.ttf"
#elif _WIN32
# define FONT_PATH "C:\\Windows\\Fonts\\Arial.ttf"
#endif

TTF_Font* Text_LoadFont(const char* path, const int size)
{
    if (!path || (path && !path[0])) path = FONT_PATH;

    TTF_Font* font = TTF_OpenFont(path, size);

    if (!font) {
        fprintf(stderr, "ERROR: Counldn't load font. %s\n", TTF_GetError());
        return NULL;
    }

    return font;
}

void Text_Render(SDL_Renderer* renderer, const Text text, TTF_Font* font, const SDL_Color color, const SDL_bool adjust_size)
{
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.str, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (color.a != 255)
        SDL_SetTextureAlphaMod(texture, color.a);

    SDL_Rect rect = {(int)text.x, (int)text.y, text.w, text.h};

    if (adjust_size)
        TTF_SizeText(font, text.str, &rect.w, &rect.h);

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}