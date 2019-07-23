#pragma once

#include "MathDefs.h"
#include <vector>
#include <sstream>
#include <limits>

namespace Math {

inline std::vector<std::string> Split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

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
template <typename T>
inline bool Equals(T lhs, T rhs)
{
    return lhs + std::numeric_limits<T>::epsilon() >= rhs && lhs - std::numeric_limits<T>::epsilon() <= rhs;
}

template <typename T>
inline bool Equals(T lhs, T rhs, T epsilon)
{
    return lhs + epsilon >= rhs && lhs - epsilon <= rhs;
}

template <typename T>
inline bool IsInfinite(T value)
{
    static_assert(std::numeric_limits<T>::has_infinity);
    return Equals(value, std::numeric_limits<T>::infinity());
}

template <typename T, typename U>
inline T Lerp(T lhs, T rhs, U i)
{
    return (i - lhs) / (rhs - lhs);
}

/// Return fractional part of passed value in range [0, 1).
template <typename T>
T Fract(T value)
{
    return value - floor(value);
}

template <typename T>
inline T DegToRad(T deg)
{
    return deg * (static_cast<T>(M_PIF / 180.0));
}

template <typename T>
inline T RadToDeg(T rad)
{
    return (rad / static_cast<T>(M_PIF)) * (static_cast<T>(180.0));
}

/// Make angle between 0 and 2 * Pi.
inline void NormalizeAngle(float& angle)
{
    angle = fmod(angle, Math::M_TWOPI);
    angle = fmod(angle + Math::M_TWOPI, Math::M_TWOPI);
}

inline float NormalizedAngle(float angle)
{
    NormalizeAngle(angle);
    return angle;
}

}
