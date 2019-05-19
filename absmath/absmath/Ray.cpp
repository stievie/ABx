#include "stdafx.h"
#include "Ray.h"
#include "MathDefs.h"
#include "BoundingBox.h"
#include "Sphere.h"
#include "Matrix4.h"

namespace Math {

Vector3 Ray::ClosestPoint(const Ray& ray) const
{
    // Algorithm based on http://paulbourke.net/geometry/lineline3d/
    const Vector3 p13 = origin_ - ray.origin_;
    const Vector3 p43 = ray.direction_;
    const Vector3 p21 = direction_;

    const float d1343 = p13.DotProduct(p43);
    const float d4321 = p43.DotProduct(p21);
    const float d1321 = p13.DotProduct(p21);
    const float d4343 = p43.DotProduct(p43);
    const float d2121 = p21.DotProduct(p21);

    const float d = d2121 * d4343 - d4321 * d4321;
    if (fabs(d) < M_EPSILON)
        return origin_;
    const float n = d1343 * d4321 - d1321 * d4343;
    const float a = n / d;

    return origin_ + a * direction_;
}

float Ray::HitDistance(const BoundingBox& box) const
{
    // If undefined, no hit (infinite distance)
    if (!box.IsDefined())
        return INFINITY;

    float dist = INFINITY;

#if (defined(HAVE_DIRECTX_MATH) || defined(HAVE_X_MATH))
    // TODO: Check!
    const XMath::BoundingOrientedBox xbb = (XMath::BoundingOrientedBox)box;
    if (xbb.Intersects(origin_, direction_, dist))
        return dist;
    else
        return INFINITY;
#else
    // Check for ray origin being inside the box
    if (box.IsInside(origin_))
        return 0.0f;

    // Check for intersecting in the X-direction
    if (origin_.x_ < box.min_.x_ && direction_.x_ > 0.0f)
    {
        float x = (box.min_.x_ - origin_.x_) / direction_.x_;
        if (x < dist)
        {
            Vector3 point = origin_ + x * direction_;
            if (point.y_ >= box.min_.y_ && point.y_ <= box.max_.y_ && point.z_ >= box.min_.z_ && point.z_ <= box.max_.z_)
                dist = x;
        }
    }
    if (origin_.x_ > box.max_.x_ && direction_.x_ < 0.0f)
    {
        float x = (box.max_.x_ - origin_.x_) / direction_.x_;
        if (x < dist)
        {
            Vector3 point = origin_ + x * direction_;
            if (point.y_ >= box.min_.y_ && point.y_ <= box.max_.y_ && point.z_ >= box.min_.z_ && point.z_ <= box.max_.z_)
                dist = x;
        }
    }
    // Check for intersecting in the Y-direction
    if (origin_.y_ < box.min_.y_ && direction_.y_ > 0.0f)
    {
        float x = (box.min_.y_ - origin_.y_) / direction_.y_;
        if (x < dist)
        {
            Vector3 point = origin_ + x * direction_;
            if (point.x_ >= box.min_.x_ && point.x_ <= box.max_.x_ && point.z_ >= box.min_.z_ && point.z_ <= box.max_.z_)
                dist = x;
        }
    }
    if (origin_.y_ > box.max_.y_ && direction_.y_ < 0.0f)
    {
        float x = (box.max_.y_ - origin_.y_) / direction_.y_;
        if (x < dist)
        {
            Vector3 point = origin_ + x * direction_;
            if (point.x_ >= box.min_.x_ && point.x_ <= box.max_.x_ && point.z_ >= box.min_.z_ && point.z_ <= box.max_.z_)
                dist = x;
        }
    }
    // Check for intersecting in the Z-direction
    if (origin_.z_ < box.min_.z_ && direction_.z_ > 0.0f)
    {
        float x = (box.min_.z_ - origin_.z_) / direction_.z_;
        if (x < dist)
        {
            Vector3 point = origin_ + x * direction_;
            if (point.x_ >= box.min_.x_ && point.x_ <= box.max_.x_ && point.y_ >= box.min_.y_ && point.y_ <= box.max_.y_)
                dist = x;
        }
    }
    if (origin_.z_ > box.max_.z_ && direction_.z_ < 0.0f)
    {
        float x = (box.max_.z_ - origin_.z_) / direction_.z_;
        if (x < dist)
        {
            Vector3 point = origin_ + x * direction_;
            if (point.x_ >= box.min_.x_ && point.x_ <= box.max_.x_ && point.y_ >= box.min_.y_ && point.y_ <= box.max_.y_)
                dist = x;
        }
    }

    return dist;
#endif
}

float Ray::HitDistance(const Sphere& sphere) const
{
    const Vector3 centeredOrigin = origin_ - sphere.center_;
    const float squaredRadius = sphere.radius_ * sphere.radius_;

    // Check if ray originates inside the sphere
    if (centeredOrigin.LengthSqr() <= squaredRadius)
        return 0.0f;

    // Calculate intersection by quadratic equation
    const float a = direction_.DotProduct(direction_);
    const float b = 2.0f * centeredOrigin.DotProduct(direction_);
    const float c = centeredOrigin.DotProduct(centeredOrigin) - squaredRadius;
    const float d = b * b - 4.0f * a * c;

    // No solution
    if (d < 0.0f)
        return INFINITY;

    // Get the nearer solution
    const float dSqrt = sqrtf(d);
    const float dist = (-b - dSqrt) / (2.0f * a);
    if (dist >= 0.0f)
        return dist;
    else
        return (-b + dSqrt) / (2.0f * a);
}

Ray Ray::Transformed(const Matrix4& transform) const
{
    Ray ret;
    ret.origin_ = transform * origin_;
    ret.direction_ = transform * direction_;
    return ret;
}

}
