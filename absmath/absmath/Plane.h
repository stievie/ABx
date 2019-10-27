#pragma once

#include "Vector3.h"

namespace Math {

class Plane
{
public:
    constexpr Plane() noexcept :
        d_(0.0f)
    { }
    constexpr Plane(const Vector3& normal, float distance) noexcept :
        normal_(normal),
        d_(distance)
    { }
    Plane(const Vector3& normal, const Vector3& point)
    {
        Define(normal, point);
    }
    Plane(const Vector3& p1, const Vector3& p2, const Vector3& p3)
    {
        Define(p1, p2, p3);
    }
    ~Plane() = default;

    /// Define from 3 vertices.
    void Define(const Vector3& v0, const Vector3& v1, const Vector3& v2);
    /// Define from a normal vector and a point on the plane.
    void Define(const Vector3& normal, const Vector3& point);

    float Distance(const Vector3& point) const { return normal_.DotProduct(point) + d_; }

    Vector3 normal_;
    float d_;
};

}
