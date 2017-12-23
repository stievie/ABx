#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

inline float DegToRad(float deg)
{
    return 2.0f * (float)M_PI * (-deg / 360.0f);
}

inline float RadToDeg(float rad)
{
    return -rad * (180.0f / (float)M_PI);
}
