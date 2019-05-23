#pragma once

namespace Math {

static constexpr float M_EPSILON = 0.000001f;              // Small value
static constexpr float M_PIF = 3.14159265358979323846f;    // PI
static constexpr float M_TWOPI = M_PIF * 2.0f;             // 2*PI
static constexpr float M_PIHALF = M_PIF / 2.0f;            // PI/2
static constexpr float M_PIFOURTH = M_PIF / 4.0f;          // PI/4

/// Intersection test result.
enum Intersection
{
    OUTSIDE,
    INTERSECTS,
    INSIDE
};

}
