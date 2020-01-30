#ifndef MATHGAME_H
#define MATHGAME_H

#include <cmath>
#include <stdlib.h>
#include "Utilities.h"

//// This header holds Maths functions that emulate the functionality of flash's


//random
//Returns 0..1
float inline fRandom()
{
    return std::ldexp(float(xoshiro_next()), -64);
}

inline int clamp(int x, int a, int b)
{
    return x < a ? a : (x > b ? b : x);
}

struct point
{
    int x = 0;
    int y = 0;
};

#endif /* MATHGAME_H */
