#include "stdafx.h"
#include "Sphere.h"

namespace Math {

void Sphere::Define(const Vector3* vertices, unsigned count)
{
    if (!count)
        return;

    Clear();
    Merge(vertices, count);
}

void Sphere::Define(const BoundingBox& box)
{
    const Vector3& min = box.min_;
    const Vector3& max = box.max_;

    Clear();
    Merge(min);
    Merge(Vector3(max.x_, min.y_, min.z_));
    Merge(Vector3(min.x_, max.y_, min.z_));
    Merge(Vector3(max.x_, max.y_, min.z_));
    Merge(Vector3(min.x_, min.y_, max.z_));
    Merge(Vector3(max.x_, min.y_, max.z_));
    Merge(Vector3(min.x_, max.y_, max.z_));
    Merge(max);
}

void Sphere::Merge(const Vector3* vertices, unsigned count)
{
    while (count--)
        Merge(*vertices++);
}

void Sphere::Merge(const BoundingBox& box)
{
    const Vector3& min = box.min_;
    const Vector3& max = box.max_;

    Merge(min);
    Merge(Vector3(max.x_, min.y_, min.z_));
    Merge(Vector3(min.x_, max.y_, min.z_));
    Merge(Vector3(max.x_, max.y_, min.z_));
    Merge(Vector3(min.x_, min.y_, max.z_));
    Merge(Vector3(max.x_, min.y_, max.z_));
    Merge(Vector3(min.x_, max.y_, max.z_));
    Merge(max);
}

Sphere Sphere::Transformed(const Matrix4& transform) const
{
    return Sphere(transform * center_, radius_);
}

bool Sphere::Collides(const BoundingBox& b2, Vector3& move) const
{
    // TODO
    return false;
}

bool Sphere::Collides(const Sphere& b2, Vector3& move) const
{
    return false;
}

bool Sphere::Collides(const ConvexHull& b2, Vector3& move) const
{
    return false;
}

Intersection Sphere::IsInside(const HeightMap& sphere) const
{
    return OUTSIDE;
}

Intersection Sphere::IsInside(const ConvexHull& sphere) const
{
    return OUTSIDE;
}

Intersection Sphere::IsInside(const BoundingBox& box) const
{
    float radiusSquared = radius_ * radius_;
    float distSquared = 0;
    float temp;
    Vector3 min = box.min_;
    Vector3 max = box.max_;

    if (center_.x_ < min.x_)
    {
        temp = center_.x_ - min.x_;
        distSquared += temp * temp;
    }
    else if (center_.x_ > max.x_)
    {
        temp = center_.x_ - max.x_;
        distSquared += temp * temp;
    }
    if (center_.y_ < min.y_)
    {
        temp = center_.y_ - min.y_;
        distSquared += temp * temp;
    }
    else if (center_.y_ > max.y_)
    {
        temp = center_.y_ - max.y_;
        distSquared += temp * temp;
    }
    if (center_.z_ < min.z_)
    {
        temp = center_.z_ - min.z_;
        distSquared += temp * temp;
    }
    else if (center_.z_ > max.z_)
    {
        temp = center_.z_ - max.z_;
        distSquared += temp * temp;
    }

    if (distSquared >= radiusSquared)
        return OUTSIDE;

    min -= center_;
    max -= center_;

    Vector3 tempVec = min; // - - -
    if (tempVec.LengthSqr() >= radiusSquared)
        return INTERSECTS;
    tempVec.x_ = max.x_; // + - -
    if (tempVec.LengthSqr() >= radiusSquared)
        return INTERSECTS;
    tempVec.y_ = max.y_; // + + -
    if (tempVec.LengthSqr() >= radiusSquared)
        return INTERSECTS;
    tempVec.x_ = min.x_; // - + -
    if (tempVec.LengthSqr() >= radiusSquared)
        return INTERSECTS;
    tempVec.z_ = max.z_; // - + +
    if (tempVec.LengthSqr() >= radiusSquared)
        return INTERSECTS;
    tempVec.y_ = min.y_; // - - +
    if (tempVec.LengthSqr() >= radiusSquared)
        return INTERSECTS;
    tempVec.x_ = max.x_; // + - +
    if (tempVec.LengthSqr() >= radiusSquared)
        return INTERSECTS;
    tempVec.y_ = max.y_; // + + +
    if (tempVec.LengthSqr() >= radiusSquared)
        return INTERSECTS;

    return INSIDE;
}

Intersection Sphere::IsInsideFast(const BoundingBox& box) const
{
    float radiusSquared = radius_ * radius_;
    float distSquared = 0;
    float temp;
    Vector3 min = box.min_;
    Vector3 max = box.max_;

    if (center_.x_ < min.x_)
    {
        temp = center_.x_ - min.x_;
        distSquared += temp * temp;
    }
    else if (center_.x_ > max.x_)
    {
        temp = center_.x_ - max.x_;
        distSquared += temp * temp;
    }
    if (center_.y_ < min.y_)
    {
        temp = center_.y_ - min.y_;
        distSquared += temp * temp;
    }
    else if (center_.y_ > max.y_)
    {
        temp = center_.y_ - max.y_;
        distSquared += temp * temp;
    }
    if (center_.z_ < min.z_)
    {
        temp = center_.z_ - min.z_;
        distSquared += temp * temp;
    }
    else if (center_.z_ > max.z_)
    {
        temp = center_.z_ - max.z_;
        distSquared += temp * temp;
    }

    if (distSquared >= radiusSquared)
        return OUTSIDE;
    else
        return INSIDE;
}

}
