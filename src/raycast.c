#include "raycast.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_ttf.h>

#include <stdint.h>
#include <math.h>
#include <stdlib.h>

#include "clock.h"
#include "color.h"
#include "map.h"
#include "text.h"
#include "textures.h"

#include <time.h>

/* PRIVATE FUNCTIONS */

SDL_bool _autotex_generation(
    Texture** floor_tex,
    Texture** ceiling_tex,
    TexGroup** wall_tex,
    uint8_t flags)
{
    SDL_bool tex_generate = SDL_FALSE;

    if (flags & (AUTO_FLOOR_TEX | AUTO_FULL_TEX))
    {
        *floor_tex = malloc(sizeof(Texture));
        (*floor_tex)->w = 64, (*floor_tex)->h = 64;

        (*floor_tex)->pixels = malloc(sizeof(Pixel) * (*floor_tex)->w * (*floor_tex)->h);

        for(int x = 0; x < (*floor_tex)->w; x++) for(int y = 0; y < (*floor_tex)->h; y++) {
            const int xor_c = XORCOLOR(x, y, (*floor_tex)->w, (*floor_tex)->h);
            (*floor_tex)->pixels[y * (*floor_tex)->w + x] = ((128 + xor_c * 65536) + xor_c) * (x % 16 && y % 16);
        }

        tex_generate = SDL_TRUE;
    }

    if (flags & (AUTO_CEILING_TEX | AUTO_FULL_TEX))
    {
        *ceiling_tex = malloc(sizeof(Texture));
        (*ceiling_tex)->w = 64, (*ceiling_tex)->h = 64;

        (*ceiling_tex)->pixels = malloc(sizeof(Pixel) * (*ceiling_tex)->w * (*ceiling_tex)->h);

        for(int x = 0; x < (*ceiling_tex)->w; x++) for(int y = 0; y < (*ceiling_tex)->h; y++) {
            (*ceiling_tex)->pixels[y * (*ceiling_tex)->w + x] = 65536 + 192 * (x % (*ceiling_tex)->w && y % (*ceiling_tex)->h);
        }

        tex_generate = SDL_TRUE;
    }

    if (flags & (AUTO_WALL_TEX | AUTO_FULL_TEX))
    {
        *wall_tex = malloc(sizeof(TexGroup));

        (*wall_tex)->length = 10;
        (*wall_tex)->w = 64, (*wall_tex)->h = 64;

        (*wall_tex)->pixels = malloc(sizeof(*(*wall_tex)->pixels) * (*wall_tex)->length);

        for (unsigned i = 0; i < (*wall_tex)->length; ++i) {
            (*wall_tex)->pixels[i] = malloc(sizeof((*wall_tex)->pixels[0]) * (*wall_tex)->w * (*wall_tex)->h);
        }

        for(int x = 0; x < (*wall_tex)->w; x++) for(int y = 0; y < (*wall_tex)->h; y++)
        {
            const int xor_c = XORCOLOR(x,y,(*wall_tex)->w,(*wall_tex)->h);
            const int xy_c = XYGRADIENT(x,y,(*wall_tex)->w,(*wall_tex)->h);
            const int x_c = XGRADIENT(x,(*wall_tex)->w), y_c = YGRADIENT(y,(*wall_tex)->h);
            (*wall_tex)->pixels[0][y * (*wall_tex)->w + x] = 65536 * 254 * (x != y && x != (*wall_tex)->w - y);         // flat red texture with black cross
            (*wall_tex)->pixels[1][y * (*wall_tex)->w + x] = xy_c + 256 * xy_c + 65536 * xy_c;                          // sloped greyscale
            (*wall_tex)->pixels[2][y * (*wall_tex)->w + x] = 256 * xy_c + 65536 * xy_c;                                 // sloped yellow gradient
            (*wall_tex)->pixels[3][y * (*wall_tex)->w + x] = xor_c + 256 * xor_c + 65536 * xor_c;                       // xor greyscale
            (*wall_tex)->pixels[4][y * (*wall_tex)->w + x] = 256 * xor_c;                                               // xor green
            (*wall_tex)->pixels[5][y * (*wall_tex)->w + x] = 65536 * 192 * (x % 16 && y % 16);                          // red bricks (bicks 16x16)
            (*wall_tex)->pixels[6][y * (*wall_tex)->w + x] = 65536 * y_c;                                               // red gradient
            (*wall_tex)->pixels[7][y * (*wall_tex)->w + x] = 128 + 256 * 128 + 65536 * 128;                             // flat grey texture
            (*wall_tex)->pixels[8][y * (*wall_tex)->w + x] = ((64 * x_c + 65536 * y_c) + xor_c) * (x % 32 && y % 32);   // less clear "synthwave" wall 1
            (*wall_tex)->pixels[9][y * (*wall_tex)->w + x] = ((128 * x_c + 65536 * y_c) + xor_c) * (x % 32 && y % 32);  // clearer "synthwave" wall 2
        }

        tex_generate = SDL_TRUE;
    }

    return tex_generate;
}

/* UPDATE functions */

void _update_player_movement(Raycast_Data* raycast, const Clock* clock)
{
    /* Adjust the speed according to delta time to keep a constant movement */

    const float mov_speed = !raycast->ctrl.crouch ? 5.f * clock->delta : 2.5f * clock->delta; // we go half as fast if we are crouched

    /* Get direction taking into account speed when moving diagonally */

    float vx = 0.f, vy = 0.f;

    if (raycast->ctrl.up)    vy -= 1.f;
    if (raycast->ctrl.down)  vy += 1.f;
    if (raycast->ctrl.left)  vx -= 1.f;
    if (raycast->ctrl.right) vx += 1.f;

    const float length = sqrt(vx * vx + vy * vy);
    if (length > 0) vx /= length, vy /= length;

    float new_pos_x, new_pos_y;

    if (fabsf(vy) > 0)
    {
        new_pos_x = raycast->pos_x - (raycast->dir_x * mov_speed) * vy;
        new_pos_y = raycast->pos_y - (raycast->dir_y * mov_speed) * vy;

        if(!raycast->map->data[(int)(new_pos_x)][(int)(raycast->pos_y)])
            raycast->pos_x = new_pos_x;

        if(!raycast->map->data[(int)(raycast->pos_x)][(int)(new_pos_y)])
            raycast->pos_y = new_pos_y;
    }

    if (fabsf(vx) > 0)
    {
        new_pos_x = raycast->pos_x + (raycast->plane_x * mov_speed) * vx;
        new_pos_y = raycast->pos_y + (raycast->plane_y * mov_speed) * vx;

        if(!raycast->map->data[(int)(new_pos_x)][(int)(raycast->pos_y)])
            raycast->pos_x = new_pos_x;

        if(!raycast->map->data[(int)(raycast->pos_x)][(int)(new_pos_y)])
            raycast->pos_y = new_pos_y;
    }

    /* Jump and crouch */

    if(raycast->ctrl.jump)
    {
        raycast->jump_phase += 0.0765f;                        // += speed
        raycast->pos_z += cosf(raycast->jump_phase) * 30;   // cos * height

        if (raycast->pos_z < 0) {
            raycast->ctrl.jump = SDL_FALSE;
            raycast->jump_phase = 0;
            raycast->pos_z = 0;
        }
    }

    if(raycast->ctrl.crouch)
    {
        if (raycast->pos_z > -200)
            raycast->pos_z -= 300 * mov_speed; // we crouch faster than we get up
        else raycast->pos_z = -200;
    }
    else
    {
        if(raycast->pos_z > 0) raycast->pos_z = fmaxf(0, raycast->pos_z - 100 * mov_speed);
        if(raycast->pos_z < 0) raycast->pos_z = fminf(0, raycast->pos_z + 100 * mov_speed);
    }
}

void _update_player_camera(Raycast_Data* raycast, const Clock* clock)
{
    /* Checking camera movements with the mouse */

    float old_dir_x, old_plane_x;

    if (raycast->ctrl.mouse_mx)
    {
        raycast->ctrl.mouse_mx = SDL_FALSE;

        const float rot_speed = .5f * abs(raycast->ctrl.mouse_dx) * clock->delta;

        if (raycast->ctrl.mouse_dx < 0) // Look to left
        {
            old_dir_x = raycast->dir_x;
            raycast->dir_x = raycast->dir_x * cos(rot_speed) - raycast->dir_y * sin(rot_speed);
            raycast->dir_y = old_dir_x * sin(rot_speed) + raycast->dir_y * cos(rot_speed);

            old_plane_x = raycast->plane_x;
            raycast->plane_x = raycast->plane_x * cos(rot_speed) - raycast->plane_y * sin(rot_speed);
            raycast->plane_y = old_plane_x * sin(rot_speed) + raycast->plane_y * cos(rot_speed);
        }
        else if (raycast->ctrl.mouse_dx > 0) // Look to right
        {
            old_dir_x = raycast->dir_x;
            raycast->dir_x = raycast->dir_x * cos(-rot_speed) - raycast->dir_y * sin(-rot_speed);
            raycast->dir_y = old_dir_x * sin(-rot_speed) + raycast->dir_y * cos(-rot_speed);

            old_plane_x = raycast->plane_x;
            raycast->plane_x = raycast->plane_x * cos(-rot_speed) - raycast->plane_y * sin(-rot_speed);
            raycast->plane_y = old_plane_x * sin(-rot_speed) + raycast->plane_y * cos(-rot_speed);
        }
    }

    if (raycast->ctrl.mouse_my)
    {
        raycast->ctrl.mouse_my = SDL_FALSE;

        const float cam_z_speed = .5f * abs(raycast->ctrl.mouse_dy) * clock->delta;

        if (raycast->ctrl.mouse_dy < 0) // Look up
        {
            raycast->pitch += 400 * cam_z_speed;
            if(raycast->pitch > 200) raycast->pitch = 200;
        }
        else if (raycast->ctrl.mouse_dy > 0) // Look down
        {
            raycast->pitch -= 400 * cam_z_speed;
            if(raycast->pitch < -200) raycast->pitch = -200;
        }
    }

    // To reset the view (up/down).
    // if (raycast->pitch > 0) raycast->pitch = fmaxf(0, raycast->pitch - 100 * rot_speed);
    // if (raycast->pitch < 0) raycast->pitch = fminf(0, raycast->pitch + 100 * rot_speed);

}

/* RAYCASTING and RENDERING or BUFFERING functions */

void _casting_textured_floor_ceiling(Raycast_Data* raycast) // floor and ceiling casting only for textured mode
{
    if (raycast->floor_tex || raycast->ceiling_tex) // TEXTURED MODE
    {
        for(int y = 0; y < raycast->win_h; ++y)
        {
            // whether this section is floor or ceiling
            const SDL_bool is_floor = y > raycast->h_win_h + raycast->pitch;

            // rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
            const float ray_dir_x0 = raycast->dir_x - raycast->plane_x;
            const float ray_dir_y0 = raycast->dir_y - raycast->plane_y;
            const float ray_dir_x1 = raycast->dir_x + raycast->plane_x;
            const float ray_dir_y1 = raycast->dir_y + raycast->plane_y;

            // Current y position compared to the center of the screen (the horizon)
            const int p = is_floor ? (y - raycast->h_win_h - raycast->pitch) : (raycast->h_win_h - y + raycast->pitch);

            // Vertical position of the camera.
            // NOTE: with 0.5, it's exactly in the center between floor and ceiling,
            // matching also how the walls are being raycasted. For different values
            // than 0.5, a separate loop must be done for ceiling and floor since
            // they're no longer symmetrical.
            const float cam_z = is_floor ? (0.5 * raycast->win_h + raycast->pos_z) : (0.5 * raycast->win_h - raycast->pos_z);

            // Horizontal distance from the camera to the floor for the current row.
            // 0.5 is the z position exactly in the middle between floor and ceiling.
            // NOTE: this is affine texture mapping, which is not perspective correct
            // except for perfectly horizontal and vertical surfaces like the floor.
            // NOTE: this formula is explained as follows: The camera ray goes through
            // the following two points: the camera itself, which is at a certain
            // height (raycast->pos_z), and a point in front of the camera (through an imagined
            // vertical plane containing the screen pixels) with horizontal distance
            // 1 from the camera, and vertical position p lower than raycast->pos_z (raycast->pos_z - p). When going
            // through that point, the line has vertically traveled by p units and
            // horizontally by 1 unit. To hit the floor, it instead needs to travel by
            // raycast->pos_z units. It will travel the same ratio horizontally. The ratio was
            // 1 / p for going through the camera plane, so to go raycast->pos_z times farther
            // to reach the floor, we get that the total horizontal distance is raycast->pos_z / p.
            const float row_dist = cam_z / p;

            // calculate the real world step vector we have to add for each x (parallel to camera plane)
            // adding step by step avoids multiplications with a weight in the inner loop
            const float floor_ceiling_step_x = row_dist * (ray_dir_x1 - ray_dir_x0) / raycast->win_w;
            const float floor_ceiling_step_y = row_dist * (ray_dir_y1 - ray_dir_y0) / raycast->win_w;

            // real world coordinates of the leftmost column. This will be updated as we step to the right.
            float floor_ceiling_x = raycast->pos_x + row_dist * ray_dir_x0;
            float floor_ceiling_y = raycast->pos_y + row_dist * ray_dir_y0;

            for(int x = 0; x < raycast->win_w; ++x)
            {
                // the cell coord is simply got from the integer parts of floor_x and floor_y
                const int cell_x = (int)(floor_ceiling_x);
                const int cell_y = (int)(floor_ceiling_y);

                uint32_t color;

                if(is_floor) // floor
                {
                    if (raycast->floor_tex) // get the texture coordinate from the fractional part
                    {
                        const int tx = (int)(raycast->floor_tex->w * (floor_ceiling_x - cell_x)) & (raycast->floor_tex->w - 1);
                        const int ty = (int)(raycast->floor_tex->h * (floor_ceiling_y - cell_y)) & (raycast->floor_tex->h - 1);
                        color = raycast->floor_tex->pixels[ty * raycast->floor_tex->w + tx] >> 1 & 8355711; // get pixel and make a bit darker
                    }
                    else color = 0x007B00; // if there is no texture, we apply a default color
                }
                else // ceiling
                {
                    if (raycast->ceiling_tex) // get the texture coordinate from the fractional part
                    {
                        const int tx = (int)(raycast->ceiling_tex->w * (floor_ceiling_x - cell_x)) & (raycast->ceiling_tex->w - 1);
                        const int ty = (int)(raycast->ceiling_tex->h * (floor_ceiling_y - cell_y)) & (raycast->ceiling_tex->h - 1);
                        color = raycast->ceiling_tex->pixels[ty * raycast->ceiling_tex->w + tx] >> 1 & 8355711; // get pixel and make a bit darker
                    }
                    else color = 0x003FFF; // if there is no texture, we apply a default color
                }

                // write in buffer
                raycast->buffer[y * raycast->win_w + x] = color;

                // step increment
                floor_ceiling_x += floor_ceiling_step_x;
                floor_ceiling_y += floor_ceiling_step_y;
            }
        }
    }
    else // TEXTURED WITHOUT FLOOR AND CEILING
    {
        for(int x = 0; x < raycast->win_w; x++) {
            for(int y = raycast->h_win_h - 1 + raycast->pitch; y < raycast->win_h ; y++) // floor
                raycast->buffer[y * raycast->win_w + x] = 0x007B00;
            for(int y = raycast->h_win_h - 1 - raycast->pitch; y < raycast->win_h ; y++) // ceiling
                raycast->buffer[(raycast->win_h - y - 1)* raycast->win_w + x] = 0x003FFF;
        }
    }
}

void _render_colored_floor_ceiling(SDL_Renderer* renderer, Raycast_Data* raycast) // floor and ceiling rendering for colored mode
{
    // Draw floor
    SDL_SetRenderDrawColor(renderer,
        0, 123, 0, 255
    );
    SDL_RenderFillRect(renderer,
        &(const SDL_Rect){ 0,
            raycast->h_win_h + raycast->pitch,
            raycast->win_w,
            raycast->win_h
        }
    );

    // Draw sky
    SDL_SetRenderDrawColor(renderer,
        0, 63, 255, 255
    );
    SDL_RenderFillRect(renderer,
        &(const SDL_Rect){ 0, 0,
        raycast->win_w,
        raycast->h_win_h + raycast->pitch
        }
    );
}

void _casting_walls(SDL_Renderer* renderer, Raycast_Data* raycast) // this function (in addition to casting) buffers for textured mode or directly renders for colored mode
{
    for (unsigned x = 0; x < raycast->win_w; x++)
    {
        /* calculate ray position and direction */

        const float camera_x = 2 * x / (float)(raycast->win_w) - 1; //x-coordinate in camera space
        const float ray_dir_x = raycast->dir_x + raycast->plane_x * camera_x;
        const float ray_dir_y = raycast->dir_y + raycast->plane_y * camera_x;

        /* which box of the map we're in */

        int on_map_pos_x = (int)(raycast->pos_x);
        int on_map_pos_y = (int)(raycast->pos_y);

        /* length of ray from current position to next x or y-side */

        float side_dist_x, side_dist_y;

        /* length of ray from one x or y-side to next x or y-side */

        const float delta_dist_x = (ray_dir_x == 0) ? 1e30 : fabsf(1 / ray_dir_x);
        const float delta_dist_y = (ray_dir_y == 0) ? 1e30 : fabsf(1 / ray_dir_y);
        float perp_wall_dist;

        /* what direction to step in x or y-direction (either +1 or -1) */
    
        int step_x, step_y;

        /* hit: was there a wall hit? & side: was a NS or a EW wall hit? */

        int hit = 0, side;

        /* calculate step and initial side_dist */

        if (ray_dir_x < 0)
        {
            step_x = -1;
            side_dist_x = (raycast->pos_x - on_map_pos_x) * delta_dist_x;
        }
        else
        {
            step_x = 1;
            side_dist_x = (on_map_pos_x + 1.f - raycast->pos_x) * delta_dist_x;
        }
        if (ray_dir_y < 0)
        {
            step_y = -1;
            side_dist_y = (raycast->pos_y - on_map_pos_y) * delta_dist_y;
        }
        else
        {
            step_y = 1;
            side_dist_y = (on_map_pos_y + 1.f - raycast->pos_y) * delta_dist_y;
        }

        /* perform DDA to find the index of squares colliding with the ray  */

        while (hit == 0)
        {
            /* jump to next map square, either in x-direction, or in y-direction */

            if (side_dist_x < side_dist_y)
            {
                side_dist_x += delta_dist_x;
                on_map_pos_x += step_x;
                side = 0;
            }
            else
            {
                side_dist_y += delta_dist_y;
                on_map_pos_y += step_y;
                side = 1;
            }

            /* Check if ray has hit a wall */

            if (raycast->map->data[on_map_pos_x][on_map_pos_y] > 0) hit = 1;
        }

        /* Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!) */

        if (side == 0) perp_wall_dist = side_dist_x - delta_dist_x;
        else           perp_wall_dist = side_dist_y - delta_dist_y;

        /* Calculate height of line to draw on screen */

        const int line_height = (int)(raycast->win_h / perp_wall_dist);

        /* calculate lowest and highest pixel to fill in current stripe */

        int draw_start = -line_height / 2.f + raycast->h_win_h + raycast->pitch + (raycast->pos_z / perp_wall_dist);
        if (draw_start < 0) draw_start = 0;

        int draw_end = line_height / 2.f + raycast->h_win_h + raycast->pitch + (raycast->pos_z / perp_wall_dist);
        if (draw_end >= raycast->win_h) draw_end = raycast->win_h - 1;

        if (raycast->wall_tex) // TEXTURED MODE
        {
            /* texturing calculations */

            const uint8_t tex_num = raycast->map->data[on_map_pos_x][on_map_pos_y] - 1; // 1 subtracted from it so that texture 0 can be used !

            /* calculate value of wall_x */

            float wall_x; // where exactly the wall was hit
            if (side == 0) wall_x = raycast->pos_y + perp_wall_dist * ray_dir_y;
            else           wall_x = raycast->pos_x + perp_wall_dist * ray_dir_x;
            wall_x -= floor(wall_x);

            /* x coordinate on the texture */

            int tex_x = (int)(wall_x * (float)(raycast->wall_tex->w));
            if(side == 0 && ray_dir_x > 0) tex_x = raycast->wall_tex->w - tex_x - 1;
            if(side == 1 && ray_dir_y < 0) tex_x = raycast->wall_tex->w - tex_x - 1;

            /* How much to increase the texture coordinate per screen pixel */

            const float step = (float)raycast->wall_tex->h / line_height;

            /* Starting texture coordinate */ // tex_y first initialize in float (tex_pos) for precision during the addition of step.

            float tex_pos = (draw_start - raycast->pitch - (raycast->pos_z / perp_wall_dist) - raycast->h_win_h + line_height / 2.f) * step;

            for(int y = draw_start; y < draw_end+1; y++) // BUG: If we are stuck to a wall at spawn, the side where you are stuck is not displayed.
            {
                /* Cast the texture coordinate to integer, and mask with (tex_h - 1) in case of overflow */

                const int tex_y = (int)tex_pos; tex_pos += step;
                uint32_t color = raycast->wall_tex->pixels[tex_num][tex_y * raycast->wall_tex->w + tex_x];

                /* make color darker for y-sides: R, G and B byte each divided through two with a "shift" and an "and" */

                if(side == 1) color = (color >> 1) & 8355711;
                raycast->buffer[y * raycast->win_w + x] = color;
            }
        }
        else // COLORED MODE
        {
            /* Apply color according to the side of the wall and draw line */

            RGB color;

            for (unsigned i = 0; i < raycast->map->wall_num; i++)
                if (raycast->map->data[on_map_pos_x][on_map_pos_y] == i+1) {
                    memcpy(color, raycast->map->wall_color[i], sizeof(RGB));
                    break;
                }

            if (side == 1) for (unsigned i = 0; i < 3; i++)
                if (color[i] > 0) color[i] /= 2;

            SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], 255);
            SDL_RenderDrawLine(renderer, x, draw_start, x, draw_end);
        }
    }
}

void _render_buffer(SDL_Renderer* renderer, Raycast_Data* raycast) // this function renders the buffer for the textured mode
{
    SDL_UpdateTexture(raycast->tex_render, NULL, raycast->buffer, raycast->win_w * sizeof(uint32_t));
    SDL_RenderCopy(renderer, raycast->tex_render, NULL, NULL);

    for(int x = 0; x < raycast->win_w; x++) {
        for(int y = 0; y < raycast->win_h; y++) {
            raycast->buffer[y * raycast->win_w + x] = 0;
        }
    }
}

void _render_map(SDL_Renderer* renderer, const Raycast_Data* raycast)
{
    const int tile_size = 10;
    const int map_pos_x = (raycast->win_w - raycast->map->width*tile_size) / 2;
    const int map_pos_y = (raycast->win_h - raycast->map->height*tile_size) / 2;
    const int on_map_pos_x = map_pos_x + floor(raycast->pos_x) * tile_size;
    const int on_map_pos_y = map_pos_y + floor(raycast->pos_y) * tile_size;

    Map_Render(
        raycast->map,
        renderer,
        map_pos_x,
        map_pos_y,
        tile_size,
        SDL_FALSE
    );

    const SDL_Rect player_pos = {
        on_map_pos_x, on_map_pos_y, 10, 10
    };

    SDL_RenderFillRect(renderer, &player_pos);
}

void _render_fps(SDL_Renderer* renderer, Raycast_Data* raycast, const Clock* clock)
{
    snprintf(raycast->text_frame_rate.str, 8, "FPS: %d", clock->fps);

    Text_Render(renderer,
        raycast->text_frame_rate,
        raycast->main_font,
        (SDL_Color){255,255,0,255},
        SDL_TRUE
    );
}

/* PUBLIC FUNCTIONS */

void Raycast_LoadMap(Raycast_Data* raycast, const Map* map, const uint16_t pos_x, const uint16_t pos_y)
{
    raycast->map = map;

    if (pos_x > 0 && pos_x <= map->width
     && pos_y > 0 && pos_y <= map->height) {
        raycast->pos_x = pos_x+.5f;
        raycast->pos_y = pos_y+.5f;
    } else {

        for (unsigned x = 1; x < map->width; x++) {
            for (unsigned y = 1; y < map->height; y++) {
                if (map->data[x][y] == 0) {
                    raycast->pos_x = x+.5f;
                    raycast->pos_y = y+.5f;
                    x = map->width; break;
                }
            }
        }

    }


}

void Raycast_LoadTex(
    SDL_Renderer* renderer,
    Raycast_Data* raycast,
    Texture* floor_tex,
    Texture* ceiling_tex,
    TexGroup* wall_tex)
{
    if (!raycast->buffer)
    {
        raycast->buffer = malloc(raycast->win_w * raycast->win_h * sizeof(uint32_t));

        raycast->tex_render = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            raycast->win_w, raycast->win_h
        );

        raycast->floor_tex = floor_tex;
        raycast->ceiling_tex = ceiling_tex;
        raycast->wall_tex = wall_tex;
    }
    else
    {
        fprintf(stderr, "ERROR of Raycast_LoadTex: The raycaster has already been initialized with textures.\n");
    }
}

Raycast_Data* Raycast_Init(
    SDL_Renderer* renderer,
    const uint16_t win_w,
    const uint16_t win_h,
    const Map* map,
    const uint16_t player_x,
    const uint16_t player_y,
    uint8_t flags)
{
    /* Autotex generation */

    Texture* floor_tex = NULL;
    Texture* ceiling_tex = NULL;
    TexGroup* wall_tex = NULL;

    const SDL_bool autotex = _autotex_generation(
        &floor_tex, &ceiling_tex, &wall_tex, flags
    );

    /* Raycaster init */

    Raycast_Data* raycast = malloc(sizeof(Raycast_Data));

    raycast->win_w = win_w;
    raycast->win_h = win_h;
    raycast->h_win_w = win_w / 2;
    raycast->h_win_h = win_h / 2;
    raycast->pos_x = 0.f;
    raycast->pos_y = 0.f;
    raycast->pos_z = 0.f;
    raycast->pitch = 0.f;
    raycast->dir_x = -1.f;
    raycast->dir_y = 0.f;
    raycast->plane_x = 0.f;
    raycast->plane_y = .66f;

    raycast->ctrl = (struct _Raycast_Ctrls){
        SDL_FALSE, SDL_FALSE, SDL_FALSE,
        SDL_FALSE, SDL_FALSE, SDL_FALSE,
        SDL_FALSE, SDL_FALSE, 0.f, 0.f,
        SDL_FALSE, SDL_FALSE
    };

    raycast->jump_phase = 0.f;
    raycast->crouch_phase = 0.f;

    if (autotex) {
        raycast->buffer = malloc(win_w * win_h * sizeof(uint32_t));
        raycast->tex_render = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            win_w, win_h
        );
    } else raycast->buffer = NULL, raycast->tex_render = NULL;

    raycast->floor_tex = floor_tex;
    raycast->ceiling_tex = ceiling_tex;
    raycast->wall_tex = wall_tex;

    Raycast_LoadMap(raycast, map, player_x, player_y);

    if(!TTF_WasInit()) {
        if (TTF_Init() < 0) {
            fprintf(stderr, "Error of TTF_Init: %s\n", TTF_GetError());
            exit(1);
        } raycast->ttf_was_init = SDL_FALSE;
    } else raycast->ttf_was_init = SDL_TRUE;

    raycast->main_font = Text_LoadFont(NULL, 16);
    raycast->text_frame_rate = (Text){ 0,0,0,0, NULL };
    raycast->text_frame_rate.str = malloc(8);

    /* Misc settings */

    SDL_SetRelativeMouseMode(SDL_TRUE);

    return raycast;
}

void Raycast_GetEvents(Raycast_Data* raycast, const SDL_Event* event)
{
    switch (event->type)
    {
        case SDL_KEYDOWN:

            switch (event->key.keysym.scancode) //sym for keycode
            {
                case SDL_SCANCODE_W:
                    raycast->ctrl.up = SDL_TRUE; break;
                case SDL_SCANCODE_S:
                    raycast->ctrl.down = SDL_TRUE; break;
                case SDL_SCANCODE_A:
                    raycast->ctrl.left = SDL_TRUE; break;
                case SDL_SCANCODE_D:
                    raycast->ctrl.right = SDL_TRUE; break;

                case SDL_SCANCODE_SPACE:
                    if (!raycast->ctrl.jump)
                        raycast->ctrl.jump = SDL_TRUE;
                    break;

                case SDL_SCANCODE_LSHIFT:
                    raycast->ctrl.crouch = SDL_TRUE;
                    break;

                case SDL_SCANCODE_F1:
                    raycast->ctrl.map_display = !raycast->ctrl.map_display;
                    break;

                case SDL_SCANCODE_F3:
                    raycast->ctrl.fps_display = !raycast->ctrl.fps_display;
                    break;

                default:
                    break;
            }

            break;

        case SDL_KEYUP:

            switch (event->key.keysym.scancode) //sym for keycode
            {
                case SDL_SCANCODE_W:
                    raycast->ctrl.up = SDL_FALSE; break;
                case SDL_SCANCODE_S:
                    raycast->ctrl.down = SDL_FALSE; break;
                case SDL_SCANCODE_A:
                    raycast->ctrl.left = SDL_FALSE; break;
                case SDL_SCANCODE_D:
                    raycast->ctrl.right = SDL_FALSE; break;

                case SDL_SCANCODE_LSHIFT:
                    raycast->ctrl.crouch = SDL_FALSE;
                    break;

                default:
                    break;
            }

            break;

        case SDL_MOUSEMOTION:

            if (!raycast->ctrl.mouse_mx && abs(event->motion.xrel) > 0) {
                raycast->ctrl.mouse_dx = event->motion.xrel;
                raycast->ctrl.mouse_mx = SDL_TRUE;
            }

            if (!raycast->ctrl.mouse_my && abs(event->motion.yrel) > 0) {
                raycast->ctrl.mouse_dy = event->motion.yrel;
                raycast->ctrl.mouse_my = SDL_TRUE;
            }

            break;

        default:
            break;
    }
}

void Raycast_Update(Raycast_Data* raycast, const Clock* clock)
{
    _update_player_movement(raycast, clock);
    _update_player_camera(raycast, clock);
}

void Raycast_Render(Raycast_Data* raycast, SDL_Renderer* renderer, const Clock* clock)
{
    if (raycast->buffer) {
        _casting_textured_floor_ceiling(raycast);
        _casting_walls(NULL, raycast);
        _render_buffer(renderer, raycast);
    }
    else {
        _render_colored_floor_ceiling(renderer, raycast);
        _casting_walls(renderer, raycast);
    }

    if (raycast->ctrl.map_display)
        _render_map(renderer, raycast);

    if (raycast->ctrl.fps_display)
        _render_fps(renderer, raycast, clock);
}

void Raycast_Free(Raycast_Data* raycast)
{
    if (raycast->wall_tex)
        TexGroup_Destroy((TexGroup*)raycast->wall_tex);

    if (raycast->ceiling_tex)
        Texture_Free((Texture*)raycast->ceiling_tex);

    if (raycast->floor_tex)
        Texture_Free((Texture*)raycast->floor_tex);

    free(raycast->text_frame_rate.str);

    TTF_CloseFont(raycast->main_font);
    if (!raycast->ttf_was_init) TTF_Quit();

    SDL_DestroyTexture(raycast->tex_render);
    free(raycast->buffer);

    free(raycast);
}