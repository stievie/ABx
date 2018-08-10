#pragma once

namespace Math {

static constexpr float M_EPSILON = 0.000001f;

/// Intersection test result.
enum Intersection
{
    OUTSIDE,
    INTERSECTS,
    INSIDE
};

}
