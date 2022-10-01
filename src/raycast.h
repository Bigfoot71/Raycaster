#ifndef _RAYCAST_H_
#define _RAYCAST_H_

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_ttf.h>
#include <stdint.h>

#include "clock.h"
#include "map.h"
#include "textures.h"
#include "text.h"

#define COLORED                 0x00
#define AUTO_WALL_TEX           0x01
#define AUTO_FLOOR_TEX          0x02
#define AUTO_CEILING_TEX        0x04
#define AUTO_FULL_TEX           0x08

struct _Raycast_Ctrls {
    SDL_bool up, down;
    SDL_bool left, right;
    SDL_bool jump, crouch;
    SDL_bool mouse_mx, mouse_my;
    int8_t mouse_dx, mouse_dy;
    SDL_bool map_display;
    SDL_bool fps_display;
}; 

// raycast -> pos_z: vertical camera strafing up/down, for jumping/crouching. 0 means standard height. Expressed in screen pixels a wall at distance 1 shifts.
// raycast -> pitch: looking up/down, expressed in screen pixels the horizon shifts.
// raycast -> mouse_mX|Y: whether the mouse moves on an X or Y axis.

typedef struct {

    uint16_t win_w, win_h;
    uint16_t h_win_w, h_win_h;

    float pos_x, pos_y;
    float pos_z, pitch; 
    float dir_x, dir_y;
    float plane_x, plane_y;

    struct _Raycast_Ctrls ctrl;
    float jump_phase, crouch_phase;

    uint32_t* buffer;
    SDL_Texture* tex_render;

    const Texture* floor_tex;
    const Texture* ceiling_tex;
    const TexGroup* wall_tex;

    const Map* map;

    SDL_bool ttf_was_init;
    TTF_Font* main_font;
    Text text_frame_rate;

} Raycast_Data;

void Raycast_LoadMap(
    Raycast_Data* raycast,
    const Map* map,
    const uint16_t pos_x,
    const uint16_t pos_y
);

void Raycast_LoadTex(
    SDL_Renderer* renderer,
    Raycast_Data* raycast,
    Texture* floor_tex,
    Texture* ceiling_tex,
    TexGroup* wall_tex
);

Raycast_Data* Raycast_Init(
    SDL_Renderer* renderer,
    const uint16_t win_w,
    const uint16_t win_h,
    const Map* map,
    const uint16_t player_x,
    const uint16_t player_y,
    uint8_t flags
);

void Raycast_GetEvents(
    Raycast_Data* raycast,
    const SDL_Event* event
);

void Raycast_Update(
    Raycast_Data* raycast,
    const Clock* clock
);

void Raycast_Render(
    Raycast_Data* raycast,
    SDL_Renderer* renderer,
    const Clock* clock
);

void Raycast_Free(
    Raycast_Data* raycast
);

#endif