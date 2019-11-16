#pragma once

#include <array>

namespace Math {

constexpr float M_EPSILON = 0.000001f;              // Small value
constexpr float M_PIF = 3.14159265358979323846f;    // PI
constexpr float M_TWOPI = M_PIF * 2.0f;             // 2*PI
constexpr float M_PIHALF = M_PIF / 2.0f;            // PI/2
constexpr float M_PIFOURTH = M_PIF / 4.0f;          // PI/4
constexpr float M_INFINITE = std::numeric_limits<float>::infinity();

/// Intersection test result.
enum class Intersection
{
    Outside,
    Intersects,
    Inside
};

using STLVector3 = std::array<float, 3>;
using STLVector4 = std::array<float, 4>;

}
