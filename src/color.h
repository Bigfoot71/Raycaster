#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>

#define XGRADIENT(X,W)  (X * 256 / W)
#define YGRADIENT(Y,H)  XGRADIENT(Y,H)
#define XYGRADIENT(X,Y,W,H) (Y * 128 / H + X * 128 / W)
#define XORCOLOR(X,Y,W,H)   ((X * 256 / W) ^ (Y * 256 / H))

typedef uint8_t RGB[3];
typedef RGB RGB_Array[];

#endif