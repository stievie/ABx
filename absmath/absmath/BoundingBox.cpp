#include "stdafx.h"
#include "BoundingBox.h"
#include "HeightMap.h"
#include "ConvexHull.h"
#include "Sphere.h"
#include "Shape.h"
#include "Gjk.h"
#include "Matrix4.h"
#include "Logger.h"

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
    orientation_ = Quaternion::Identity;
}

BoundingBox BoundingBox::Transformed(const Matrix4& transform) const
{
    if (!IsDefined())
        // No transformation needed when not defined
        return *this;

    const Vector3 newCenter = transform * Center();
    const Vector3 oldEdge = Size() * 0.5f;

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

    BoundingBox result = BoundingBox(newCenter - newEdge, newCenter + newEdge);
    result.orientation_ = orientation_;
    return result;
}

Shape BoundingBox::GetShape() const
{
    Shape s;
    const Vector3& boundPoint1 = min_;
    const Vector3& boundPoint2 = max_;
    const Vector3 boundPoint3 = Vector3(boundPoint1.x_, boundPoint1.y_, boundPoint2.z_);
    const Vector3 boundPoint4 = Vector3(boundPoint1.x_, boundPoint2.y_, boundPoint1.z_);
    const Vector3 boundPoint5 = Vector3(boundPoint2.x_, boundPoint1.y_, boundPoint1.z_);
    const Vector3 boundPoint6 = Vector3(boundPoint1.x_, boundPoint2.y_, boundPoint2.z_);
    const Vector3 boundPoint7 = Vector3(boundPoint2.x_, boundPoint1.y_, boundPoint2.z_);
    const Vector3 boundPoint8 = Vector3(boundPoint2.x_, boundPoint2.y_, boundPoint1.z_);

    s.vertexData_.push_back(boundPoint1);
    s.vertexData_.push_back(boundPoint2);
    s.vertexData_.push_back(boundPoint3);
    s.vertexData_.push_back(boundPoint4);
    s.vertexData_.push_back(boundPoint5);
    s.vertexData_.push_back(boundPoint6);
    s.vertexData_.push_back(boundPoint7);
    s.vertexData_.push_back(boundPoint8);
    s.vertexCount_ = 8;
    s.indexCount_ = 0;

    return s;
}

bool BoundingBox::Collides(const BoundingBox& b2) const
{
#if defined(HAVE_DIRECTX_MATH) || defined(HAVE_X_MATH)
    const bool o1 = IsOriented();
    const bool o2 = b2.IsOriented();
    if (o1 && o2)
        return ((XMath::BoundingOrientedBox)*this).Contains((XMath::BoundingOrientedBox)b2) != XMath::DISJOINT;
    else if (o1)
        return ((XMath::BoundingOrientedBox)*this).Contains((XMath::BoundingBox)b2) != XMath::DISJOINT;
    else if (o2)
        return ((XMath::BoundingBox)*this).Contains((XMath::BoundingOrientedBox)b2) != XMath::DISJOINT;
    else
        return ((XMath::BoundingBox)*this).Contains((XMath::BoundingBox)b2) != XMath::DISJOINT;
#else
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
#endif
}

bool BoundingBox::Collides(const BoundingBox& b2, Vector3& move) const
{
#if defined(HAVE_DIRECTX_MATH) || defined(HAVE_X_MATH)
    const bool o1 = IsOriented();
    const bool o2 = b2.IsOriented();
    if (o1 && o2)
    {
        if (((XMath::BoundingOrientedBox)*this).Contains((XMath::BoundingOrientedBox)b2) == XMath::DISJOINT)
            return false;
    }
    else if (o1)
    {
        if (((XMath::BoundingOrientedBox)*this).Contains((XMath::BoundingBox)b2) == XMath::DISJOINT)
            return false;
    }
    else if (o2)
    {
        if (((XMath::BoundingBox)*this).Contains((XMath::BoundingOrientedBox)b2) == XMath::DISJOINT)
            return false;
    }
#endif
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
#if 0
    move.y_ = abs(top) < bottom ? top : bottom;
#else
    // Y is taken from the terrain, so only X and Z direction is possible
    move.y_ = 0.0f;
#endif
    move.z_ = abs(front) < back ? front : back;

#if 0
    // return the smallest
    if (abs(move.x_) <= abs(move.y_) && abs(move.x_) <= abs(move.z_))
    {
        // X Largest
        move.y_ = 0.0f;
        move.z_ = 0.0f;
    }
    else if (abs(move.y_) <= abs(move.z_))
    {
        move.x_ = 0.0f;
        move.z_ = 0.0f;
    }
    else
    {
        move.x_ = 0.0f;
        move.y_ = 0.0f;
    }
#else
    if (abs(move.x_) <= abs(move.z_))
    {
        // X Largest
        move.z_ = 0.0f;
    }
    else
    {
        move.x_ = 0.0f;
    }
#endif
    return true;
}

bool BoundingBox::Collides(const Sphere& b2) const
{
    if (IsOriented())
    {
        return ((XMath::BoundingOrientedBox)*this).Contains((XMath::BoundingSphere)b2) > XMath::DISJOINT;
    }
    return IsInside(b2) != OUTSIDE;
}

bool BoundingBox::Collides(const Sphere& b2, Vector3&) const
{
    if (IsOriented())
    {
        return ((XMath::BoundingOrientedBox)*this).Contains((XMath::BoundingSphere)b2) > XMath::DISJOINT;
    }
    return IsInside(b2) != OUTSIDE;
}

bool BoundingBox::Collides(const ConvexHull& b2, Vector3&) const
{
    return IsInside(b2) != OUTSIDE;
}

bool BoundingBox::Collides(const HeightMap& b2, Vector3& move) const
{
    Vector3 centerBottom = Center();
    centerBottom.y_ -= Extends().y_;
    const float y = b2.GetHeight(centerBottom);
    if (y > min_.y_)
    {
        move.y_ = min_.y_ - y;
        return true;
    }
    return false;
}

Intersection BoundingBox::IsInside(const HeightMap& shape) const
{
    Vector3 centerBottom = Center();
    centerBottom.y_ -= Extends().y_;
    const float y = shape.GetHeight(centerBottom);
    if (y > min_.y_)
    {
        return INSIDE;
    }
    return OUTSIDE;
}

Intersection BoundingBox::IsInside(const ConvexHull& shape) const
{
    const Shape s = GetShape();

    if (Gjk::StaticIntersects(s, shape))
        return INSIDE;
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
