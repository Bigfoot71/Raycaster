#ifndef _TEXTURES_H_
#define _TEXTURES_H_

#include <stdint.h>

typedef uint32_t Pixel;

typedef struct {
    uint16_t w, h;
    Pixel* pixels;
} Texture;

typedef struct {
    uint16_t length;
    uint16_t w, h;
    Pixel** pixels;
} TexGroup;

Texture* Texture_Load(const char* path);
void Texture_Free(Texture* tex);

TexGroup* TexGroup_Load(const char** paths, const unsigned tex_num);
void TexGroup_Destroy(TexGroup* tex_grp);

#endif