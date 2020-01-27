/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "MathDefs.h"
#include <vector>
#include <sstream>
#include <limits>
#include <cmath>

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
    return lhs + std::numeric_limits<T>::epsilon() >= rhs &&
        lhs - std::numeric_limits<T>::epsilon() <= rhs;
}

template <typename T>
inline bool Equals(T lhs, T rhs, T epsilon)
{
    return lhs + epsilon >= rhs && lhs - epsilon <= rhs;
}

template <typename T>
inline bool IsInfinite(T value)
{
    static_assert(std::numeric_limits<T>::has_infinity, "T has no infinity");
    return Equals(value, std::numeric_limits<T>::infinity());
}

template <typename T, typename U>
inline T Lerp(T lhs, T rhs, U i)
{
    return lhs * (static_cast<U>(1.0) - i) + rhs * i;
}

/// Return fractional part of passed value in range [0, 1].
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
    angle = fmodf(angle, M_TWOPI);
    angle = fmodf(angle + M_TWOPI, M_TWOPI);
}

inline float NormalizedAngle(float angle)
{
    NormalizeAngle(angle);
    return angle;
}

}
