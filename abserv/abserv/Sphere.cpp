#include "stdafx.h"
#include "Sphere.h"
#include "Shape.h"
#include "ConvexHull.h"
#include "Gjk.h"
#include "HeightMap.h"

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

Shape Sphere::GetShape() const
{
    // http://www.andrewnoske.com/wiki/Generating_a_sphere_as_a_3D_mesh
    Shape s;
    static const Vector3 v[12] = {
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

    for (unsigned i = 0; i < 12; i++)
    {
        s.vertexData_.push_back(Vector3(
            v[i] * radius_ + center_
        ));
    }
    s.vertexCount_ = 12;
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
    return Sphere(transform * center_, radius_);
}

bool Sphere::Collides(const BoundingBox& b2, Vector3& move) const
{
    return b2.Collides(*this, move);
}

bool Sphere::Collides(const Sphere& b2, Vector3& move) const
{
    AB_UNUSED(move);
    const Shape s = b2.GetShape();

    Gjk gjk;
    if (gjk.Intersects(this->GetShape(), s))
        return true;
    return false;
}

bool Sphere::Collides(const ConvexHull& b2, Vector3& move) const
{
    return b2.Collides(*this, move);
}

bool Sphere::Collides(const HeightMap& b2, Vector3& move) const
{
    BoundingBox bbox = GetBoundingBox();
    return bbox.Collides(b2, move);
}

Intersection Sphere::IsInside(const HeightMap& sphere) const
{
    Vector3 move;
    return Collides(sphere, move) ? INSIDE : OUTSIDE;
}

Intersection Sphere::IsInside(const ConvexHull& sphere) const
{
    Vector3 move;
    return Collides(sphere, move) ? INSIDE : OUTSIDE;
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
