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

#include "Vector3.h"
#include "Plane.h"
#include "Matrix4.h"
#include "MathDefs.h"

namespace Math {

class BoundingBox;
class Shape;
class Sphere;

class Frustum
{
public:
    enum FrustumPlanes
    {
        PlaneNear = 0,
        PlaneLeft,
        PlaneRight,
        PlaneUp,
        PlaneDown,
        PlaneFar,
        CountPlanes
    };
private:
    void UpdatePlanes();
public:
    constexpr Frustum() noexcept
    { }
    Frustum(const Frustum& other) noexcept;

    ~Frustum() = default;

    void Define(float fov, float aspectRatio, float zoom,
        float nearZ, float farZ,
        const Matrix4& transform = Matrix4::Identity);
    void Define(const Vector3& _near, const Vector3& _far,
        const Matrix4& transform = Matrix4::Identity);

    Frustum Transformed(const Matrix4& transform) const;

    Intersection IsInside(const BoundingBox& bbox);
    /// Test if a point is inside or outside.
    Intersection IsInside(const Vector3& point) const
    {
        for (const auto& plane : planes_)
        {
            if (plane.Distance(point) < 0.0f)
                return Intersection::Outside;
        }

        return Intersection::Inside;
    }
    /// Test if a sphere is inside, outside or intersects.
    Intersection IsInside(const Sphere& sphere) const;
    /// Return distance of a point to the frustum, or 0 if inside.
    float Distance(const Vector3& point) const
    {
        float distance = 0.0f;
        for (const auto& plane : planes_)
            distance = std::max(-plane.Distance(point), distance);

        return distance;
    }

    Shape GetShape() const;

    Vector3 vertices_[8];
    Plane planes_[FrustumPlanes::CountPlanes];
};

}
