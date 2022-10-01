#include "clock.h"

#include <SDL2/SDL_timer.h>
#include <stdint.h>

Clock Clock_Init(void)
{
    Clock clock;
    clock.last_tick = 0;
    clock.delta_ms  = 0;
    clock.delta     = 0;
    clock.t_fps     = 0;
    clock.fps       = 0;
    return clock;
}

void Clock_Update(Clock* clock)
{
    uint32_t act_tick = SDL_GetTicks();
    clock->delta_ms   = act_tick - clock->last_tick;
    clock->last_tick  = act_tick;

    clock->delta = clock->delta_ms / 1000.f;
    clock->t_fps += clock->delta;

    if (clock->t_fps > 1)
    {
        clock->fps = 1.f / clock->delta;
        clock->t_fps = 0.f;
    }
}

void Clock_Limit(Clock *clock)
{
    if (SDL_GetTicks() < (clock->last_tick + TPF))
        SDL_Delay((clock->last_tick + TPF) - SDL_GetTicks());
}
