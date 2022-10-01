#include "textures.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_surface.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <string.h>
#include <time.h>

Texture* Texture_Load(const char* path)
{
    Texture* tex = malloc(sizeof(Texture));

    SDL_Surface* tex_surface_tmp = IMG_Load(path);

    if (!tex_surface_tmp) {
        fprintf(stderr, "Error of IMG_Load: %s\n", IMG_GetError());
        exit(1);
    }

    SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
    SDL_Surface* tex_surface = SDL_ConvertSurface(tex_surface_tmp, format, 0);
    SDL_FreeSurface(tex_surface_tmp);

    const size_t tex_size = sizeof(Pixel) * tex_surface->w * tex_surface->h;

    tex->pixels = malloc(tex_size);
    memcpy(tex->pixels, tex_surface->pixels, tex_size);

    tex->w = tex_surface->w;
    tex->h = tex_surface->h;

    SDL_FreeSurface(tex_surface);

    return tex;
}

void Texture_Free(Texture* tex)
{
    free(tex->pixels);
    free(tex);
}

TexGroup* TexGroup_Load(const char** paths, const unsigned tex_num)
{
        TexGroup* tex_grp = malloc(sizeof(TexGroup));
        tex_grp->pixels = malloc(sizeof(*tex_grp->pixels) * tex_num);
        tex_grp->length = tex_num;

        uint16_t tex_w[2], tex_h[2];
        size_t tex_size;

        for (unsigned i = 0; i < tex_num; i++)
        {
            SDL_Surface* tex_surface_tmp = IMG_Load(paths[i]);

            if (!tex_surface_tmp) {
                fprintf(stderr, "%s\n", IMG_GetError());
                exit(1);
            }

            SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGB888);
            SDL_Surface* tex_surface = SDL_ConvertSurface(tex_surface_tmp, format, 0);
            SDL_FreeSurface(tex_surface_tmp);

            tex_w[0] = tex_surface->w, tex_h[0] = tex_surface->h;

            if (i==0) {
                tex_size = sizeof(Pixel) * tex_w[0] * tex_h[0];
            } else if (tex_w[0] != tex_w[1] || tex_h[0] != tex_h[1]) {
                fprintf(stderr, "ERROR of Tex_Array_New: The dimensions of \"%s\" are not identical to the previous textures.\n", paths[i]);
                exit(1);
            }

            tex_grp->pixels[i] = malloc(tex_size);
            memcpy(tex_grp->pixels[i], tex_surface->pixels, tex_size);

            tex_w[1] = tex_w[0], tex_h[1] = tex_h[0];

            SDL_FreeSurface(tex_surface);
        }

        tex_grp->w = tex_w[0];
        tex_grp->h = tex_h[0];

    return tex_grp;
}

void TexGroup_Destroy(TexGroup* tex_grp)
{
    for (int i = 0; i < tex_grp->length; i++)
        free(tex_grp->pixels[i]);

    free(tex_grp->pixels);
    free(tex_grp);
}
