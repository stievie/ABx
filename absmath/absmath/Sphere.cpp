#include "stdafx.h"
#include "Sphere.h"
#include "Shape.h"
#include "ConvexHull.h"
#include "Gjk.h"
#include "HeightMap.h"
#include "BoundingBox.h"
#include "Matrix4.h"

namespace Math {

void Sphere::Define(const Vector3* vertices, unsigned count)
{
    if (!count)
        return;

    Reset();
    Merge(vertices, count);
}

void Sphere::Define(const BoundingBox& box)
{
    const Vector3& min = box.min_;
    const Vector3& max = box.max_;

    Reset();
    Merge(min);
    Merge(Vector3(max.x_, min.y_, min.z_));
    Merge(Vector3(min.x_, max.y_, min.z_));
    Merge(Vector3(max.x_, max.y_, min.z_));
    Merge(Vector3(min.x_, min.y_, max.z_));
    Merge(Vector3(max.x_, min.y_, max.z_));
    Merge(Vector3(min.x_, max.y_, max.z_));
    Merge(max);
}

void Sphere::Merge(const Vector3& point)
{
    if (radius_ < 0.0f)
    {
        center_ = point;
        radius_ = 0.0f;
        return;
    }

    const Vector3 offset = point - center_;
    const float dist = offset.Length();

    if (dist > radius_)
    {
        const float half = (dist - radius_) * 0.5f;
        radius_ += half;
        center_ += (half / dist) * offset;
    }
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

BoundingBox Sphere::GetBoundingBox() const
{
    return BoundingBox(center_ - radius_, center_ + radius_);
}

Shape Sphere::GetShape() const
{
    // http://www.andrewnoske.com/wiki/Generating_a_sphere_as_a_3D_mesh
    Shape s;
    static constexpr unsigned VERTEX_COUNT = 12;
    static constexpr Vector3 V[VERTEX_COUNT] = {
        {  0.000f,  1.000f,  0.000f },  // Top-most point.
        {  0.894f,  0.447f,  0.000f },
        {  0.276f,  0.447f,  0.851f },
        { -0.724f,  0.447f,  0.526f },
        { -0.724f,  0.447f, -0.526f },
        {  0.276f,  0.447f, -0.851f },
        {  0.724f, -0.447f,  0.526f },
        { -0.276f, -0.447f,  0.851f },
        { -0.894f, -0.447f,  0.000f },
        { -0.276f, -0.447f, -0.851f },
        {  0.724f, -0.447f, -0.526f },
        {  0.000f, -1.000f,  0.000f }   // Bottom-most point.
    };

    for (unsigned i = 0; i < VERTEX_COUNT; ++i)
    {
        s.vertexData_.push_back({ V[i] * radius_ + center_ });
    }
    s.vertexCount_ = VERTEX_COUNT;
    // https://www.csee.umbc.edu/~squire/reference/polyhedra.shtml#icosahedron
    s.AddTriangle(0, 1, 2);
    s.AddTriangle(0, 2, 3);
    s.AddTriangle(0, 3, 4);
    s.AddTriangle(0, 4, 5);
    s.AddTriangle(0, 5, 1);
    s.AddTriangle(11, 6, 7);
    s.AddTriangle(11, 7, 8);
    s.AddTriangle(11, 8, 9);
    s.AddTriangle(11, 9, 10);
    s.AddTriangle(11, 10, 6);
    s.AddTriangle(1, 2, 6);
    s.AddTriangle(2, 3, 7);
    s.AddTriangle(3, 4, 8);
    s.AddTriangle(4, 5, 9);
    s.AddTriangle(5, 1, 10);
    s.AddTriangle(6, 7, 2);
    s.AddTriangle(7, 8, 3);
    s.AddTriangle(8, 9, 4);
    s.AddTriangle(9, 10, 5);
    s.AddTriangle(10, 6, 1);
    return s;
}

Sphere Sphere::Transformed(const Matrix4& transform) const
{
    // Urho3D uses x from the size component to scale the radius.
    return Sphere(transform * center_, transform.Scaling().x_ * radius_);
}

bool Sphere::Collides(const BoundingBox& b2) const
{
    return IsInsideFast(b2) != Intersection::Outside;
}

bool Sphere::Collides(const BoundingBox& b2, const Vector3& velocity, Vector3& move) const
{
    return b2.Collides(*this, velocity, move);
}

bool Sphere::Collides(const Sphere& b2, const Vector3&, Vector3&) const
{
    const Shape s = b2.GetShape();

    if (Gjk::StaticIntersects(this->GetShape(), s))
        return true;
    return false;
}

bool Sphere::Collides(const ConvexHull& b2, const Vector3& velocity, Vector3& move) const
{
    return b2.Collides(*this, velocity, move);
}

bool Sphere::Collides(const HeightMap& b2, const Vector3& velocity, Vector3& move) const
{
    const BoundingBox bbox = GetBoundingBox();
    return bbox.Collides(b2, velocity, move);
}

Intersection Sphere::IsInside(const HeightMap& sphere) const
{
    Vector3 move;
    return Collides(sphere, Vector3::Zero, move) ? Intersection::Inside : Intersection::Outside;
}

Intersection Sphere::IsInside(const ConvexHull& sphere) const
{
    Vector3 move;
    return Collides(sphere, Vector3::Zero, move) ? Intersection::Inside : Intersection::Outside;
}

Intersection Sphere::IsInside(const BoundingBox& box) const
{
#if defined(HAVE_DIRECTX_MATH)
    if (box.IsOriented())
    {
        const XMath::BoundingSphere me(center_, radius_);
        const XMath::ContainmentType ct = me.Contains((XMath::BoundingOrientedBox)box);
        return ct == XMath::DISJOINT ? Intersection::Outside : (ct == XMath::INTERSECTS ? Intersection::Intersects : Intersection::Inside);
    }
#endif
    const float radiusSquared = radius_ * radius_;
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
        return Intersection::Outside;

    min -= center_;
    max -= center_;

    Vector3 tempVec = min; // - - -
    if (tempVec.LengthSqr() >= radiusSquared)
        return Intersection::Intersects;
    tempVec.x_ = max.x_; // + - -
    if (tempVec.LengthSqr() >= radiusSquared)
        return Intersection::Intersects;
    tempVec.y_ = max.y_; // + + -
    if (tempVec.LengthSqr() >= radiusSquared)
        return Intersection::Intersects;
    tempVec.x_ = min.x_; // - + -
    if (tempVec.LengthSqr() >= radiusSquared)
        return Intersection::Intersects;
    tempVec.z_ = max.z_; // - + +
    if (tempVec.LengthSqr() >= radiusSquared)
        return Intersection::Intersects;
    tempVec.y_ = min.y_; // - - +
    if (tempVec.LengthSqr() >= radiusSquared)
        return Intersection::Intersects;
    tempVec.x_ = max.x_; // + - +
    if (tempVec.LengthSqr() >= radiusSquared)
        return Intersection::Intersects;
    tempVec.y_ = max.y_; // + + +
    if (tempVec.LengthSqr() >= radiusSquared)
        return Intersection::Intersects;

    return Intersection::Inside;
}

Intersection Sphere::IsInsideFast(const BoundingBox& box) const
{
    if (box.IsOriented())
        return IsInside(box);

    const float radiusSquared = radius_ * radius_;
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
        return Intersection::Outside;
    else
        return Intersection::Inside;
}

}
