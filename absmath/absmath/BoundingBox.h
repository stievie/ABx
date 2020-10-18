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
#include "MathDefs.h"
#include "Quaternion.h"
#include <array>
#include "MathConfig.h"

namespace Math {

class ConvexHull;
class HeightMap;
class Sphere;
class Shape;
class Matrix4;
class Plane;
class Line;

class BoundingBox
{
private:
    enum Orientations : uint32_t
    {
        OrientationsNone = 0,
        OrientationsO1 = 1 << 1,
        OrientationsO2 = 1 << 2
    };
    uint32_t GetOrientations(const BoundingBox& b2) const
    {
        uint32_t o = OrientationsNone;
        if (IsOriented())
            o |= OrientationsO1;
        if (b2.IsOriented())
            o |= OrientationsO2;
        return o;
    }
public:
    constexpr BoundingBox() noexcept :
        min_(Vector3(Math::M_INFINITE, Math::M_INFINITE, Math::M_INFINITE)),
        max_(Vector3(-Math::M_INFINITE, -Math::M_INFINITE, -Math::M_INFINITE)),
        orientation_(Quaternion::Identity)
    { }
    constexpr BoundingBox(const BoundingBox& other) noexcept :
        min_(other.min_),
        max_(other.max_),
        orientation_(other.orientation_)
    { }
    constexpr BoundingBox(const Vector3& min, const Vector3& max) noexcept :
        min_(min),
        max_(max),
        orientation_(Quaternion::Identity)
    { }
    constexpr BoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) noexcept :
        min_(minX, minY, minZ),
        max_(maxX, maxY, maxZ),
        orientation_(Quaternion::Identity)
    { }
    /// Construct from minimum and maximum floats (all dimensions same.)
    constexpr BoundingBox(float min, float max) noexcept :
        min_(min, min, min),
        max_(max, max, max),
        orientation_(Quaternion::Identity)
    { }
    ~BoundingBox() = default;

    BoundingBox& operator = (const BoundingBox& other) = default;

#if defined(HAVE_DIRECTX_MATH)
    operator XMath::BoundingBox() const
    {
        return XMath::BoundingBox(Center(), Extends());
    }
    operator XMath::BoundingOrientedBox() const
    {
        return XMath::BoundingOrientedBox(Center(), Extends(), orientation_);
    }
#endif
    bool operator ==(const BoundingBox& rhs) const { return Equals(rhs); }
    bool operator !=(const BoundingBox& rhs) const { return !Equals(rhs); }
    inline bool Equals(const BoundingBox& rhs) const
    {
        return min_.Equals(rhs.min_) && max_.Equals(rhs.max_);
    }

    void Define(float min, float max);

    Vector3 Center() const { return (max_ + min_) * 0.5; }
    Vector3 Size() const { return max_ - min_; }
    BoundingBox GetBoundingBox() const
    {
        return *this;
    }
    std::array<Vector3, 8> GetCorners() const;
    std::array<Vector3, 4> GetCorners2D() const;
    Vector3 GetClosestCorner2D(const Vector3& pos) const;
    std::vector<Plane> GetPlanes() const;
    std::vector<Line> GetEdges() const;
    void Merge(float x, float y, float z);
    void Merge(const Vector3& vertex);
    void Merge(const Vector3* vertices, size_t count);
    void Reset();
    void AddSize(float value);

    float Width() const
    {
        return max_.x_ - min_.x_;
    }
    float Height() const
    {
        return max_.y_ - min_.y_;
    }
    float Depth() const
    {
        return max_.z_ - min_.z_;
    }
    /// Distance from the center to each side, or half size
    Vector3 Extends() const
    {
        return (max_ - min_) * 0.5f;
    }

    bool IsDefined() const { return !Math::IsInfinite(min_.x_); }
    bool IsOriented() const { return !orientation_.Equals(Quaternion::Identity); }
    std::string ToString() const
    {
        if (!IsDefined())
            return "Undefined";
        std::stringstream ss;
        ss << "min: " << min_.ToString() << " ";
        ss << "max: " << max_.ToString();
        if (IsOriented())
            ss << " o: " << orientation_.ToString();
        return ss.str();
    }

    /// AABB
    BoundingBox Transformed(const Matrix4& transform) const;

    Shape GetShape() const;

    /// https://www.gamedev.net/resources/_/technical/game-programming/swept-aabb-collision-detection-and-response-r3084
    /// Returns true if the boxes are colliding (velocities are not used)
    bool Collides(const BoundingBox& b2) const;
    /// Returns true if the boxes are colliding (velocities are not used)
    /// move will return the movement the b1 must move to avoid the collision
    bool Collides(const BoundingBox& b2, const Vector3&, Vector3& move) const;
    bool Collides(const Sphere& b2) const;
    bool Collides(const Sphere& b2, const Vector3& velocity, Vector3& move) const;
    bool Collides(const ConvexHull& b2, const Vector3& velocity, Vector3& move) const;
    bool Collides(const HeightMap& b2, const Vector3& velocity, Vector3& move) const;

    /// Test if a point is inside.
    Intersection IsInside(const Vector3& point) const
    {
        if (point.x_ < min_.x_ || point.x_ > max_.x_ || point.y_ < min_.y_ || point.y_ > max_.y_ ||
            point.z_ < min_.z_ || point.z_ > max_.z_)
            return Intersection::Outside;

        return Intersection::Inside;
    }

    /// Test if another bounding box is inside, outside or intersects.
    Intersection IsInside(const BoundingBox& box) const
    {
        if (IsOriented() || box.IsOriented())
        {
#if defined(HAVE_DIRECTX_MATH)
            XMath::BoundingOrientedBox me = static_cast<XMath::BoundingOrientedBox>(*this);
            XMath::ContainmentType ct = me.Contains(static_cast<XMath::BoundingOrientedBox>(box));
            return ct == XMath::DISJOINT ? Intersection::Outside : (ct == XMath::INTERSECTS ? Intersection::Intersects : Intersection::Inside);
#else
#error DirectXMath needed
#endif
        }

        if (box.max_.x_ < min_.x_ || box.min_.x_ > max_.x_ || box.max_.y_ < min_.y_ || box.min_.y_ > max_.y_ ||
            box.max_.z_ < min_.z_ || box.min_.z_ > max_.z_)
            return Intersection::Outside;

        if (box.min_.x_ < min_.x_ || box.max_.x_ > max_.x_ || box.min_.y_ < min_.y_ || box.max_.y_ > max_.y_ ||
            box.min_.z_ < min_.z_ || box.max_.z_ > max_.z_)
            return Intersection::Intersects;

        return Intersection::Inside;
    }
    Intersection IsInside(const HeightMap& shape) const;
    Intersection IsInside(const ConvexHull& shape) const;
    Intersection IsInside(const Sphere& shape) const;

    /// Test if another bounding box is (partially) inside or outside.
    Intersection IsInsideFast(const BoundingBox& box) const
    {
        if (box.IsOriented())
            return IsInside(box);

        if (box.max_.x_ < min_.x_ || box.min_.x_ > max_.x_ || box.max_.y_ < min_.y_ || box.min_.y_ > max_.y_ ||
            box.max_.z_ < min_.z_ || box.min_.z_ > max_.z_)
            return Intersection::Outside;

        return Intersection::Inside;
    }

    friend std::ostream& operator << (std::ostream& os, const BoundingBox& value)
    {
        return os << value.ToString();
    }

    Vector3 min_;
    float dummyMin_{ 0.0f };
    Vector3 max_;
    float dummyMax_{ 0.0f };
    Quaternion orientation_;
};

}
