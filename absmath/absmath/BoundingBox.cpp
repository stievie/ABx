#include "stdafx.h"
#include "BoundingBox.h"
#include "HeightMap.h"
#include "ConvexHull.h"
#include "Sphere.h"
#include "Shape.h"
#include "Gjk.h"
#include "Matrix4.h"
#include <array>
#include "Plane.h"
#include "Line.h"

namespace Math {

void BoundingBox::Define(float min, float max)
{
    min_ = Vector3(min, min, min);
    max_ = Vector3(max, max, max);
}

std::array<Vector3, 8> BoundingBox::GetCorners() const
{
    std::array<Vector3, 8> result;
    const Vector3& boundPoint1 = min_;
    const Vector3& boundPoint2 = max_;
    const Vector3 boundPoint3 = Vector3(boundPoint1.x_, boundPoint1.y_, boundPoint2.z_);
    const Vector3 boundPoint4 = Vector3(boundPoint1.x_, boundPoint2.y_, boundPoint1.z_);
    const Vector3 boundPoint5 = Vector3(boundPoint2.x_, boundPoint1.y_, boundPoint1.z_);
    const Vector3 boundPoint6 = Vector3(boundPoint1.x_, boundPoint2.y_, boundPoint2.z_);
    const Vector3 boundPoint7 = Vector3(boundPoint2.x_, boundPoint1.y_, boundPoint2.z_);
    const Vector3 boundPoint8 = Vector3(boundPoint2.x_, boundPoint2.y_, boundPoint1.z_);

    result[0] = boundPoint1;
    result[1] = boundPoint2;
    result[2] = boundPoint3;
    result[3] = boundPoint4;
    result[4] = boundPoint5;
    result[5] = boundPoint6;
    result[6] = boundPoint7;
    result[7] = boundPoint8;

    return result;
}

std::vector<Plane> BoundingBox::GetPlanes() const
{
    // Transformed (OBB)
    const Vector3 c = Center();
    const Vector3 e = Extends();
    const Matrix4 orientation = orientation_.GetMatrix();
    const Vector3 a[] = {
        Vector3(orientation.m_[Matrix4::Index00], orientation.m_[Matrix4::Index01], orientation.m_[Matrix4::Index02]),
        Vector3(orientation.m_[Matrix4::Index10], orientation.m_[Matrix4::Index11], orientation.m_[Matrix4::Index12]),
        Vector3(orientation.m_[Matrix4::Index20], orientation.m_[Matrix4::Index21], orientation.m_[Matrix4::Index22])
    };

    std::vector<Plane> result;
    result.resize(6);
    result[0] = Plane(a[0],          a[0].DotProduct(c + a[0] * e.x_));
    result[1] = Plane(a[0] * -1.0f, -a[0].DotProduct(c - a[0] * e.x_));
    result[2] = Plane(a[1],          a[1].DotProduct(c + a[1] * e.y_));
    result[3] = Plane(a[1] * -1.0f, -a[1].DotProduct(c - a[1] * e.y_));
    result[4] = Plane(a[2],          a[2].DotProduct(c + a[2] * e.z_));
    result[5] = Plane(a[2] * -1.0f, -a[2].DotProduct(c - a[2] * e.z_));

    return result;
}

std::vector<Line> BoundingBox::GetEdges() const
{
    // Not transformed (AABB)
    std::vector<Line> result;
    result.reserve(12);
    const auto v = GetCorners();

    /*
        5-----12-----1 max
      7 |          8 |
    3------3-----7   9
    |   |        |   |
    1   2-----10-4---6
    | 5          | 6
min 0------2-----4
    */

    const int index[][2] = {
        { 0, 3 },{ 0, 4 },{ 3, 7 },{ 4, 7 },{ 0, 2 },{ 4, 6 },
        { 3, 5 },{ 7, 1 },{ 1, 6 },{ 6, 2 },{ 2, 5 },{ 5, 1 }
    };
    for (int j = 0; j < 12; ++j)
    {
        result.push_back(Line(
            v[index[j][0]], v[index[j][1]]
        ));
    }
    return result;
}

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
#if defined(HAVE_DIRECTX_MATH) || defined(HAVE_X_MATH)
    XMath::BoundingBox _result;
    ((XMath::BoundingBox)*this).Transform(_result, transform);
    BoundingBox result(Vector3(_result.Center) - Vector3(_result.Extents), Vector3(_result.Center) + Vector3(_result.Extents));
    result.orientation_ = orientation_;
    return result;
#else
    const Vector3 newCenter = transform * Center();
    const Vector3 oldEdge = Extends();

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
#endif
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

    /*
        5------------1 max
      / |          / |
    3------------7   |
    |   |        |   |
    |   2--------|---6
    | /          | /
min 0------------4
    */
    // Front
    s.AddTriangle(7, 4, 0);
    s.AddTriangle(0, 3, 7);
    // Left
    s.AddTriangle(0, 3, 2);
    s.AddTriangle(2, 5, 3);
    // Right
    s.AddTriangle(4, 7, 1);
    s.AddTriangle(1, 6, 4);
    // Up
    s.AddTriangle(7, 1, 5);
    s.AddTriangle(5, 3, 7);
    // Down
    s.AddTriangle(2, 6, 4);
    s.AddTriangle(4, 0, 2);
    // Back
    s.AddTriangle(6, 2, 5);
    s.AddTriangle(5, 1, 6);

    return s;
}

bool BoundingBox::Collides(const BoundingBox& b2) const
{
#if defined(HAVE_DIRECTX_MATH) || defined(HAVE_X_MATH)
    if (GetOrientations(b2) != OrientationsNone)
        return ((XMath::BoundingOrientedBox)*this).Intersects((XMath::BoundingOrientedBox)b2);
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

bool BoundingBox::ResolveCollision(const BoundingBox& /* b2 */, Vector3& /* result */) const
{
    // TODO: Implement
    return true;
}

bool BoundingBox::Collides(const BoundingBox& b2, Vector3& move) const
{
#if defined(HAVE_DIRECTX_MATH) || defined(HAVE_X_MATH)
    const uint32_t o = GetOrientations(b2);
    if (o != OrientationsNone)
    {
        bool result = false;
        switch (o)
        {
        case OrientationsO2:
        {
            XMath::BoundingOrientedBox xb2(b2);
            result = ((XMath::BoundingBox)*this).Intersects(xb2);
            break;
        }
        default:
            // Only AABB (this) vs. OBB is possible
            assert(false);
            return false;
        }
        if (result)
        {
            result = ResolveCollision(b2, move);
        }
        return result;
    }
#endif

    const Vector3 size1 = Size();
    const Vector3 size2 = b2.Size();

    const float left = b2.min_.x_ - (min_.x_ + size1.x_);
    const float right = (b2.min_.x_ + size2.x_) - min_.x_;
    const float top = b2.min_.y_ - (min_.y_ + size1.y_);
    const float bottom = (b2.min_.y_ + size2.y_) - min_.y_;
    const float front = b2.min_.z_ - (min_.z_ + size1.z_);
    const float back = (b2.min_.z_ + size2.z_) - min_.z_;

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
