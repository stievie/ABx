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

namespace Math {

class HeightMap;
class ConvexHull;
class Shape;
class BoundingBox;
class Matrix4;

class Sphere
{
public:
    constexpr Sphere() noexcept :
        center_(Vector3::Zero),
        radius_(-Math::M_INFINITE)
    {}
    /// Copy-construct from another sphere.
    constexpr Sphere(const Sphere& sphere) noexcept :
        center_(sphere.center_),
        radius_(sphere.radius_)
    {}
    /// Construct from center and radius.
    constexpr Sphere(const Vector3& center, float radius) noexcept :
        center_(center),
        radius_(radius)
    {}

    ~Sphere() = default;

#if defined(HAVE_DIRECTX_MATH)
    operator XMath::BoundingSphere() const
    {
        return XMath::BoundingSphere({ center_.x_, center_.y_, center_.z_ }, radius_);
    }
#endif

    /// Define from another sphere.
    void Define(const Sphere& sphere) noexcept
    {
        Define(sphere.center_, sphere.radius_);
    }

    /// Define from center and radius.
    void Define(const Vector3& center, float radius) noexcept
    {
        center_ = center;
        radius_ = radius;
    }

    /// Define from an array of vertices.
    void Define(const Vector3* vertices, unsigned count);
    /// Define from a bounding box.
    void Define(const BoundingBox& box);

    /// Merge a point.
    void Merge(const Vector3& point);
    /// Merge an array of vertices.
    void Merge(const Vector3* vertices, unsigned count);
    /// Merge a bounding box.
    void Merge(const BoundingBox& box);

    /// Clear to undefined state.
    void Reset()
    {
        center_ = Vector3::Zero;
        radius_ = -Math::M_INFINITE;
    }

    /// Return true if this sphere is defined via a previous call to Define() or Merge().
    bool IsDefined() const
    {
        return radius_ >= 0.0f;
    }

    BoundingBox GetBoundingBox() const;
    Shape GetShape() const;

    Sphere Transformed(const Matrix4& transform) const;

    bool Collides(const BoundingBox& b2) const;
    bool Collides(const BoundingBox& b2, const Vector3& velocity, Vector3& move) const;
    bool Collides(const Sphere& b2) const
    {
        return IsInsideFast(b2) != Intersection::Outside;
    }
    bool Collides(const Sphere& b2, const Vector3& velocity, Vector3&) const;
    bool Collides(const ConvexHull& b2, const Vector3& velocity, Vector3& move) const;
    bool Collides(const HeightMap& b2, const Vector3& velocity, Vector3& move) const;

    /// Test if a point is inside.
    Intersection IsInside(const Vector3& point) const
    {
        const float distSquared = (point - center_).LengthSqr();
        if (distSquared < radius_ * radius_)
            return Intersection::Inside;
        return Intersection::Outside;
    }

    /// Test if another sphere is inside, outside or intersects.
    Intersection IsInside(const Sphere& sphere) const
    {
        const float dist = (sphere.center_ - center_).Length();
        if (dist >= sphere.radius_ + radius_)
            return Intersection::Outside;
        if (dist + sphere.radius_ < radius_)
            return Intersection::Inside;
        return Intersection::Intersects;
    }
    Intersection IsInside(const HeightMap& sphere) const;
    Intersection IsInside(const ConvexHull& sphere) const;

    /// Test if another sphere is (partially) inside or outside.
    Intersection IsInsideFast(const Sphere& sphere) const
    {
        const float distSquared = (sphere.center_ - center_).LengthSqr();
        const float combined = sphere.radius_ + radius_;

        if (distSquared >= combined * combined)
            return Intersection::Outside;
        return Intersection::Inside;
    }

    /// Test if a bounding box is inside, outside or intersects.
    Intersection IsInside(const BoundingBox& box) const;
    /// Test if a bounding box is (partially) inside or outside.
    Intersection IsInsideFast(const BoundingBox& box) const;

    /// Return distance of a point to the surface, or 0 if inside.
    float Distance(const Vector3& point) const { return std::max((point - center_).Length() - radius_, 0.0f); }

    /// Sphere center.
    Vector3 center_;
    /// Sphere radius.
    float radius_;
};

template<class _Stream>
inline _Stream& operator << (_Stream& os, Sphere& value)
{
    os << value.center_.ToString() << " +/- " << value.radius_;
    return os;
}

}
