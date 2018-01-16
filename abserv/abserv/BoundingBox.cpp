#include "stdafx.h"
#include "BoundingBox.h"
#include "HeightMap.h"
#include "ConvexHull.h"
#include "Sphere.h"

namespace Math {

void BoundingBox::Merge(float x, float y, float z)
{
    if (min_.x_ > x)
        min_.x_ = x;
    if (min_.y_ > y)
        min_.y_ = y;
    if (min_.z_ > z)
        min_.z_ = z;

    if (max_.x_ < x)
        max_.x_ = x;
    if (max_.y_ < y)
        max_.y_ = y;
    if (max_.z_ < z)
        max_.z_ = z;
}

void BoundingBox::Merge(const Vector3& vertex)
{
    if (min_.x_ > vertex.x_)
        min_.x_ = vertex.x_;
    if (min_.y_ > vertex.y_)
        min_.y_ = vertex.y_;
    if (min_.z_ > vertex.z_)
        min_.z_ = vertex.z_;

    if (max_.x_ < vertex.x_)
        max_.x_ = vertex.x_;
    if (max_.y_ < vertex.y_)
        max_.y_ = vertex.y_;
    if (max_.z_ < vertex.z_)
        max_.z_ = vertex.z_;
}

void BoundingBox::Merge(const Vector3* vertices, unsigned count)
{
    while (count--)
        Merge(*vertices++);
}

void BoundingBox::Reset()
{
    min_ = { INFINITY, INFINITY, INFINITY };
    max_ = { -INFINITY, -INFINITY, -INFINITY };
}

BoundingBox BoundingBox::Transformed(const Matrix4& transform) const
{
    if (!IsDefined())
        // No transformation needed when not defined
        return *this;

    Vector3 newCenter = transform * Center();
    Vector3 oldEdge = Size() * 0.5f;

    Vector3 newEdge = Vector3(
        std::fabs(transform.m_[Matrix4::Index00]) * oldEdge.x_ +
        std::fabs(transform.m_[Matrix4::Index01]) * oldEdge.y_ +
        std::fabs(transform.m_[Matrix4::Index02]) * oldEdge.z_,

        std::fabs(transform.m_[Matrix4::Index10]) * oldEdge.x_ +
        std::fabs(transform.m_[Matrix4::Index11]) * oldEdge.y_ +
        std::fabs(transform.m_[Matrix4::Index12]) * oldEdge.z_,

        std::fabs(transform.m_[Matrix4::Index20]) * oldEdge.x_ +
        std::fabs(transform.m_[Matrix4::Index21]) * oldEdge.y_ +
        std::fabs(transform.m_[Matrix4::Index22]) * oldEdge.z_
    );

    return BoundingBox(newCenter - newEdge, newCenter + newEdge);
}

bool BoundingBox::Collides(const BoundingBox& b2) const
{
    const Vector3 size1 = Size();
    const Vector3 size2 = b2.Size();
    return !(
        min_.x_ + size1.x_ < b2.min_.x_ ||
        min_.x_ > b2.min_.x_ + size2.x_ ||
        min_.y_ + size1.y_ < b2.min_.y_ ||
        min_.y_ > b2.min_.y_ + size2.y_ ||
        min_.z_ + size1.z_ < b2.min_.z_ ||
        min_.z_ > b2.min_.z_ + size2.z_
        );
}

bool BoundingBox::Collides(const BoundingBox& b2, Vector3& move) const
{
    const Vector3 size1 = Size();
    const Vector3 size2 = b2.Size();

    float left = b2.min_.x_ - (min_.x_ + size1.x_);
    float right = (b2.min_.x_ + size2.x_) - min_.x_;
    float top = b2.min_.y_ - (min_.y_ + size1.y_);
    float bottom = (b2.min_.y_ + size2.y_) - min_.y_;
    float front = b2.min_.z_ - (min_.z_ + size1.z_);
    float back = (b2.min_.z_ + size2.z_) - min_.z_;

    // check that there was a collision
    if (left > 0 || right < 0 || top > 0 || bottom < 0 || front > 0 || back < 0)
        return false;

    // find the offset
    move.x_ = abs(left) < right ? left : right;
    move.y_ = abs(top) < bottom ? top : bottom;
    move.z_ = abs(front) < back ? front : back;

    // return the smallest
    if (abs(move.x_) > abs(move.y_) || abs(move.x_) > abs(move.z_))
        move.x_ = 0.0f;
    if (abs(move.y_) > abs(move.z_))
        move.y_ = 0.0f;
    else
        move.z_ = 0.0f;

    return true;
}

bool BoundingBox::Collides(const Sphere & b2) const
{
    return false;
}

bool BoundingBox::Collides(const Sphere & b2, Vector3 & move) const
{
    return false;
}

bool BoundingBox::Collides(const ConvexHull & b2, Vector3 & move) const
{
    return false;
}

Intersection BoundingBox::IsInside(const HeightMap& shape) const
{
    // TODO
    return OUTSIDE;
}

Intersection BoundingBox::IsInside(const ConvexHull& shape) const
{
    // TODO
    return OUTSIDE;
}

Intersection BoundingBox::IsInside(const Sphere& shape) const
{
    float distSquared = 0;
    float temp;
    const Vector3& center = shape.center_;

    if (center.x_ < min_.x_)
    {
        temp = center.x_ - min_.x_;
        distSquared += temp * temp;
    }
    else if (center.x_ > max_.x_)
    {
        temp = center.x_ - max_.x_;
        distSquared += temp * temp;
    }
    if (center.y_ < min_.y_)
    {
        temp = center.y_ - min_.y_;
        distSquared += temp * temp;
    }
    else if (center.y_ > max_.y_)
    {
        temp = center.y_ - max_.y_;
        distSquared += temp * temp;
    }
    if (center.z_ < min_.z_)
    {
        temp = center.z_ - min_.z_;
        distSquared += temp * temp;
    }
    else if (center.z_ > max_.z_)
    {
        temp = center.z_ - max_.z_;
        distSquared += temp * temp;
    }

    float radius = shape.radius_;
    if (distSquared >= radius * radius)
        return OUTSIDE;
    else if (center.x_ - radius < min_.x_ || center.x_ + radius > max_.x_ || center.y_ - radius < min_.y_ ||
        center.y_ + radius > max_.y_ || center.z_ - radius < min_.z_ || center.z_ + radius > max_.z_)
        return INTERSECTS;
    else
        return INSIDE;
}

}
