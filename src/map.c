#include "map.h"
#include "color.h"

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

Map* Map_Create(
    const uint16_t width,
    const uint16_t height,
    const uint8_t wall_num,
    const RGB_Array wall_colors,
    const uint8_t flags)
{
    const size_t size_wall_colors = sizeof(*wall_colors) * wall_num;

    Map* map = malloc(sizeof(Map) + size_wall_colors);

    *(int*)&map->width = width;
    *(int*)&map->height = height;
    *(uint8_t*)&map->wall_num = wall_num;

    memcpy(*(RGB_Array*)map->wall_color, wall_colors, size_wall_colors);

    uint8_t* values = calloc((width+1)*(height+1), sizeof(uint8_t));
    map->data = malloc((width+1)*sizeof(uint8_t*));

    if (flags & (MAP_FILL | MAP_RANDWALL))
    {
        for (int x = 0; x <= width; x++)
            map->data[x] = values + x * height;

        int w_start = 0, w_end = width;
        int h_start = 1, h_end = height-1;
        int line = 0;

        while (w_end != width / 2)
        {
            uint8_t wall = rand() % wall_num + 1;

            for (int x = w_start; x <= w_end; x++)
            {
                map->data[x][line] = wall;
                map->data[x][height-line] = wall;
            }

            for (int y = h_start; y <= h_end; y++)
            {
                map->data[line][y] = wall;
                map->data[width-line][y] = wall;
            }

            w_start++, w_end--;
            h_start++, h_end--;
            line++;
        }
    }
    else if (flags & MAP_FILL)
    {
        for (int x = 0; x <= width; x++)
        {
            map->data[x] = values + x * height;
            for (int y = 0; y <= height; y++)
                map->data[x][y] = 1;
        }
    }
    else
    {
        for (int x = 0; x <= width; x++)
            map->data[x] = values + x * height;

        for (int x = 0; x <= width; x++)
        {
            map->data[x][0] = 1;
            map->data[x][height] = 1;
        }

        for (int y = 1; y <= height-1; y++)
        {
            map->data[0][y] = 1;
            map->data[width][y] = 1;
        }
    }

    return map;
}

Map* Map_MazeGen(
    const uint16_t width,
    const uint16_t height,
    const uint8_t wall_num,
    const RGB_Array wall_colors)
{
    /* TODO NOTE: 
        This algorithm has been adapted from another of my projects,
        it would be necessary to rewrite one that will be more suitable.
    */

    Map* map = Map_Create(width+1, height+1, wall_num, wall_colors, MAP_FILL | MAP_RANDWALL);

    uint16_t pos_x = 1, pos_y = 1;
    map->data[pos_x][pos_y] = 255;

    uint8_t dir;
    SDL_bool not_pass;
    uint8_t wall_type;

    do {

        dir = rand() % 4;

        for (int i=1; i<=wall_num; i++) {
            if ((pos_x+2 < width && map->data[pos_x+2][pos_y] == i)
            || (pos_x-2 > 0 && map->data[pos_x-2][pos_y] == i)
            || (pos_y+2 < height && map->data[pos_x][pos_y+2] == i)
            || (pos_y-2 > 0 && map->data[pos_x][pos_y-2] == i)) {
                not_pass = SDL_TRUE; wall_type = i; break;
            } else not_pass = SDL_FALSE;
        }

        if (not_pass)
        {
            switch(dir)
            {
                case 0:
                    if (pos_x+2 < width && map->data[pos_x+2][pos_y] == wall_type) {
                        pos_x = pos_x+2;
                        map->data[pos_x][pos_y] = 254;
                        map->data[pos_x-1][pos_y] = 254;
                    } break;

                case 1:
                    if (pos_x-2 > 0 && map->data[pos_x-2][pos_y] == wall_type) {
                        pos_x = pos_x-2;
                        map->data[pos_x][pos_y] = 254;
                        map->data[pos_x+1][pos_y] = 254;
                    } break;

                case 2:
                    if (pos_y+2 < height && map->data[pos_x][pos_y+2] == wall_type) {
                        pos_y = pos_y+2;
                        map->data[pos_x][pos_y] = 254;
                        map->data[pos_x][pos_y-1] = 254;
                    } break;

                case 3:
                    if (pos_y-2 > 0 && map->data[pos_x][pos_y-2] == wall_type) {
                        pos_y = pos_y-2;
                        map->data[pos_x][pos_y] = 254;
                        map->data[pos_x][pos_y+1] = 254;
                    } break;

                default:
                    break;
            }

        }
        else if (map->data[pos_x+1][pos_y] == 254
              || map->data[pos_x-1][pos_y] == 254
              || map->data[pos_x][pos_y+1] == 254
              || map->data[pos_x][pos_y-1] == 254)
        {
            map->data[pos_x][pos_y] = 0;

            switch (dir)
            {
                case 0:
                    if (map->data[pos_x+1][pos_y] == 254) {
                        pos_x = pos_x+2;
                        map->data[pos_x][pos_y] = 0;
                        map->data[pos_x-1][pos_y] = 0;
                    } break;

                case 1:
                    if (map->data[pos_x-1][pos_y] == 254) {
                        pos_x = pos_x-2;
                        map->data[pos_x][pos_y] = 0;
                        map->data[pos_x+1][pos_y] = 0;
                    } break;

                case 2:
                    if (map->data[pos_x][pos_y+1] == 254) {
                        pos_y = pos_y+2;
                        map->data[pos_x][pos_y] = 0;
                        map->data[pos_x][pos_y-1] = 0;
                    } break;

                case 3:
                    if (map->data[pos_x][pos_y-1] == 254) {
                        pos_y = pos_y-2;
                        map->data[pos_x][pos_y] = 0;
                        map->data[pos_x][pos_y+1] = 0;
                    } break;

                default:
                    break;
            }
        }

    } while (map->data[1][1] != 0);

    return map;
}

Map* Map_RandGen(
    const uint16_t width,
    const uint16_t height,
    const uint8_t wall_num,
    const RGB_Array wall_colors)
{
    Map* map = Map_Create(width+1, height+1, wall_num, wall_colors, MAP_FILL | MAP_RANDWALL);

    uint16_t pos_x = 1, pos_y = 1;
    uint16_t prev_pos_x = pos_x;
    uint16_t prev_pos_y = pos_y;
    uint8_t dir; uint16_t mov;

    while (map->data[width][height] != 0)
    {
        map->data[pos_x][pos_y] = 0;

        do {

            dir = rand() % 4;

            switch(dir)
            {
                case 0: // West
                    mov = pos_x - 1;
                    if (mov > 0)
                        pos_x = mov;
                    break;
                case 1: // East
                    mov = pos_x + 1;
                    if (mov <= width)
                        pos_x = mov;
                    break;
                case 2: // North
                    mov = pos_y - 1;
                    if (mov > 0)
                        pos_y = mov;
                    break;
                case 3: // South
                    mov = pos_y + 1;
                    if (mov <= width)
                        pos_y = mov;
                    break;
            }

        } while(pos_x == prev_pos_x && pos_y == prev_pos_y);

        prev_pos_x = pos_x, prev_pos_y = pos_y;
    }

    return map;
}

void Map_Render(
    const Map* map,
    SDL_Renderer* renderer,
    const uint16_t pos_x,
    const uint16_t pos_y,
    const uint8_t tile_size,
    const SDL_bool show_grid)
{
    SDL_Rect rect = { 0, 0, tile_size, tile_size };

    for (int x = 0; x <= map->width; x++)
        for (int y = 0; y <= map->height; y++)
        {
            rect.x = pos_x + x * tile_size;
            rect.y = pos_y + y * tile_size;

            RGB color = {63,63,63}; // floor color

            for (int i = 0; i < map->wall_num; i++)
                if (map->data[x][y] == i+1) {
                    memcpy(color, map->wall_color[i], sizeof(RGB));
                    break;
                }

            SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], 255);
            SDL_RenderFillRect(renderer, &rect);

            if (show_grid) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &rect);
            }
        }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &(SDL_Rect){ pos_x, pos_y,
            tile_size*(map->width+1), tile_size*(map->height+1) }
    );
}

void Map_Destroy(Map* map)
{
    free(*map->data);
    free(map->data);
    free(map);
}