#pragma once

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

}
