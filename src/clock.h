#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <stdint.h>

#define FPS 60

#define TPF 1000/FPS

typedef struct {
    uint32_t last_tick;
    uint32_t delta_ms;
    float delta;
    float t_fps;
    uint16_t fps;
} Clock;

Clock Clock_Init(void);
void Clock_Update(Clock* clock);
void Clock_Limit(Clock* clock);

#endif