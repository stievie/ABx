#pragma once

#include "Vector3.h"
#include "BoundingBox.h"

namespace Math {

class HeightMap;
class ConvexHull;

class Sphere
{
public:
    Sphere() :
        center_(Vector3::Zero),
        radius_(-INFINITY)
    {}
    /// Copy-construct from another sphere.
    Sphere(const Sphere& sphere) :
        center_(sphere.center_),
        radius_(sphere.radius_)
    {}
    /// Construct from center and radius.
    Sphere(const Vector3& center, float radius) :
        center_(center),
        radius_(radius)
    {}

    ~Sphere() = default;

    /// Define from another sphere.
    void Define(const Sphere& sphere)
    {
        Define(sphere.center_, sphere.radius_);
    }

    /// Define from center and radius.
    void Define(const Vector3& center, float radius)
    {
        center_ = center;
        radius_ = radius;
    }

    /// Define from an array of vertices.
    void Define(const Vector3* vertices, unsigned count);
    /// Define from a bounding box.
    void Define(const BoundingBox& box);

    /// Merge a point.
    void Merge(const Vector3& point)
    {
        if (radius_ < 0.0f)
        {
            center_ = point;
            radius_ = 0.0f;
            return;
        }

        Vector3 offset = point - center_;
        float dist = offset.Length();

        if (dist > radius_)
        {
            float half = (dist - radius_) * 0.5f;
            radius_ += half;
            center_ += (half / dist) * offset;
        }
    }

    /// Merge an array of vertices.
    void Merge(const Vector3* vertices, unsigned count);
    /// Merge a bounding box.
    void Merge(const BoundingBox& box);

    /// Clear to undefined state.
    void Clear()
    {
        center_ = Vector3::Zero;
        radius_ = -INFINITY;
    }

    /// Return true if this sphere is defined via a previous call to Define() or Merge().
    bool Defined() const
    {
        return radius_ >= 0.0f;
    }

    BoundingBox GetBoundingBox() const
    {
        return BoundingBox(center_ - radius_, center_ + radius_);
    }
    Sphere Transformed(const Matrix4& transform) const;

    bool Collides(const BoundingBox& b2) const
    {
        return IsInsideFast(b2) != OUTSIDE;
    }
    bool Collides(const BoundingBox& b2, Vector3& move) const;
    bool Collides(const Sphere& b2) const
    {
        return false;
    }
    bool Collides(const Sphere& b2, Vector3& move) const;

    /// Test if a point is inside.
    Intersection IsInside(const Vector3& point) const
    {
        float distSquared = (point - center_).LengthSqr();
        if (distSquared < radius_ * radius_)
            return INSIDE;
        else
            return OUTSIDE;
    }

    /// Test if another sphere is inside, outside or intersects.
    Intersection IsInside(const Sphere& sphere) const
    {
        float dist = (sphere.center_ - center_).Length();
        if (dist >= sphere.radius_ + radius_)
            return OUTSIDE;
        else if (dist + sphere.radius_ < radius_)
            return INSIDE;
        else
            return INTERSECTS;
    }
    Intersection IsInside(const HeightMap& sphere) const;
    Intersection IsInside(const ConvexHull& sphere) const;

    /// Test if another sphere is (partially) inside or outside.
    Intersection IsInsideFast(const Sphere& sphere) const
    {
        float distSquared = (sphere.center_ - center_).LengthSqr();
        float combined = sphere.radius_ + radius_;

        if (distSquared >= combined * combined)
            return OUTSIDE;
        else
            return INSIDE;
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

}
