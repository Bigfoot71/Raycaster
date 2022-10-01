#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#include "window.h"
#include "clock.h"
#include "map.h"
#include "raycast.h"
#include "textures.h"

#define WIN_W 640
#define WIN_H 480

int main(void)
{
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    if (Window_Init(&window, &renderer, "Raycaster - Demo", WIN_W, WIN_H) != 0)
        return -1;

    SDL_Event event;
    Clock clock = Clock_Init();
    srand(time(NULL));

    /* Generate map */

    RGB_Array wall_colors = {
        { 255, 0, 0 },
        { 0, 255, 0 },
        { 0, 0, 255 },
        { 255, 255, 0},
        { 255, 0, 255},
        { 0, 255, 255},
        { 127, 255, 255},
        { 255, 127, 255 },
    };

    // Map* map = Map_MazeGen(32, 32, 8, wall_colors);
    Map* map = Map_RandGen(32, 32, 8, wall_colors);

    /* Load raycaster */

    // If you load your own textures you can leave the flag at 0.
    Raycast_Data* raycast = Raycast_Init(renderer, WIN_W, WIN_H, map, 0, 0, AUTO_FLOOR_TEX | AUTO_WALL_TEX); // FLAGS: 0 - COLORED || AUTO_FULL_TEX || AUTO_WALL_TEX || AUTO_FLOOR_TEX || AUTO_CEILING_TEX

    /* // Load textures (optional)

    Texture* floor_tex = Texture_Load("/path/to/floor.png");
    Texture* ceiling_tex = Texture_Load("/path/to/ceiling.png");

    TexGroup* wall_tex = TexGroup_Load(
        (const char*[IndicateNumber]){
            "/path/to/wall_1.png",
            "/path/to/wall_2.png",
            "/path/to/wall_3.png",
            "/path/to/wall_X.png",
            "/path/to/wall_X.png",
            "/path/to/wall_X.png"
        }, IndicateNumber
    );

    Raycast_LoadTex(renderer, raycast, floor_tex, ceiling_tex, wall_tex);

    */

    /* Run program */

    SDL_bool running = SDL_TRUE;

    while (running)
    {
        Clock_Update(&clock);

        while (SDL_PollEvent(&event))
        {
            running = event.type != SDL_QUIT;
            Raycast_GetEvents(raycast, &event);
        }

        Raycast_Update(raycast, &clock);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        Raycast_Render(raycast, renderer, &clock);

        SDL_RenderPresent(renderer);

        Clock_Limit(&clock);
    }

    Raycast_Free(raycast); // Raycast_Free destroys the textures but not the active map
    Map_Destroy(map);

    Window_Quit(window, renderer);

    return 0;
}
