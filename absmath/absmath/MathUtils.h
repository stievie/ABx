#pragma once

#include "MathDefs.h"

namespace Math {

template <typename T>
inline T Clamp(T value, T min, T max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

/// Check whether two floating point values are equal within accuracy.
template <class T>
inline bool Equals(T lhs, T rhs)
{
    return lhs + std::numeric_limits<T>::epsilon() >= rhs && lhs - std::numeric_limits<T>::epsilon() <= rhs;
}

template <typename T, typename U>
inline T Lerp(T lhs, T rhs, T i)
{
    return (i - lhs) / (rhs - lhs);
}

/// Return fractional part of passed value in range [0, 1).
template <class T>
T Fract(T value)
{
    return value - floor(value);
}

template <typename T>
inline T DegToRad(T deg)
{
    return deg * (static_cast<T>(M_PI / 180.0));
}

template <typename T>
inline T RadToDeg(T rad)
{
    return (rad / static_cast<T>(M_PI)) * (static_cast<T>(180.0));
}

/// Make angle between 0 and 2 * Pi.
inline void NormalizeAngle(float& angle)
{
    // Angle should be >= 0 and < 2 * PI
    if (angle >= 2.0f * Math::M_PIF)
        angle -= 2.0f * Math::M_PIF;
    else if (angle < 0.0f)
        angle += 2.0f * Math::M_PIF;
}

}
