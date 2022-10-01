#ifndef _MAP_H_
#define _MAP_H_

#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <stdint.h>

#include "color.h"

#define MAP_FILL     0x01
#define MAP_RANDWALL 0x02

typedef struct {
    uint8_t** data;
    const uint16_t width;
    const uint16_t height;
    const uint8_t wall_num;
    const RGB_Array wall_color;
} Map;

Map* Map_Create(
    const uint16_t width,
    const uint16_t height,
    const uint8_t wall_num,
    const RGB_Array wall_colors,
    const uint8_t flags
);

Map* Map_MazeGen(
    const uint16_t width,
    const uint16_t height,
    const uint8_t wall_num,
    const RGB_Array wall_colors
);

Map* Map_RandGen(
    const uint16_t width,
    const uint16_t height,
    const uint8_t wall_num,
    const RGB_Array wall_colors
);

void Map_Render(
    const Map* map,
    SDL_Renderer* renderer,
    const uint16_t pos_x,
    const uint16_t pos_y,
    const uint8_t tile_size,
    const SDL_bool show_grid
);

void Map_Destroy(Map* map);

#endif