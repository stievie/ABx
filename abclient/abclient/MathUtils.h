#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

inline void NormalizeAngle(float& angle)
{
    angle = fmodf(angle, 360.0f);
    angle = fmodf(angle + 360.0f, 360.0f);
}

template <typename T>
inline T DegToRad(T deg)
{
    return -deg * (static_cast<T>(M_PI / 180.0));
}

template <typename T>
inline T RadToDeg(T rad)
{
    return (-rad / static_cast<T>(M_PI)) * (static_cast<T>(180.0));
}
