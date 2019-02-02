#include "stdafx.h"
#include "Frustum.h"
#include "BoundingBox.h"
#include "Shape.h"
#include "Sphere.h"

namespace Math {

Frustum::Frustum(const Frustum& other) noexcept
{
    for (unsigned i = 0; i < 8; ++i)
        vertices_[i] = other.vertices_[i];
    UpdatePlanes();
}

void Frustum::UpdatePlanes()
{
    planes_[PlaneNear].Define(vertices_[2], vertices_[1], vertices_[0]);
    planes_[PlaneLeft].Define(vertices_[3], vertices_[7], vertices_[6]);
    planes_[PlaneRight].Define(vertices_[1], vertices_[5], vertices_[4]);
    planes_[PlaneUp].Define(vertices_[0], vertices_[4], vertices_[7]);
    planes_[PlaneDown].Define(vertices_[6], vertices_[5], vertices_[1]);
    planes_[PlaneFar].Define(vertices_[5], vertices_[6], vertices_[7]);

    // Check if we ended up with inverted planes (reflected transform) and flip in that case
    if (planes_[PlaneNear].Distance(vertices_[5]) < 0.0f)
    {
        for (unsigned i = 0; i < CountPlanes; ++i)
        {
            planes_[i].normal_ = -planes_[i].normal_;
            planes_[i].d_ = -planes_[i].d_;
        }
    }
}

void Frustum::Define(float fov, float aspectRatio, float zoom,
    float nearZ, float farZ, const Matrix4 & transform)
{
    nearZ = std::max<float>(nearZ, 0.0f);
    farZ = std::max<float>(farZ, nearZ);
    float halfViewSize = tanf(fov * static_cast<float>(M_PI / 360.0f)) / zoom;
    Vector3 _near, _far;

    _near.z_ = nearZ;
    _near.y_ = _near.z_ * halfViewSize;
    _near.x_ = _near.y_ * aspectRatio;
    _far.z_ = farZ;
    _far.y_ = _far.z_ * halfViewSize;
    _far.x_ = _far.y_ * aspectRatio;

    Define(_near, _far, transform);
}

void Frustum::Define(const Vector3& _near, const Vector3& _far,
    const Matrix4& transform)
{
    vertices_[0] = transform * _near;
    vertices_[1] = transform * Vector3(_near.x_, -_near.y_, _near.z_);
    vertices_[2] = transform * Vector3(-_near.x_, -_near.y_, _near.z_);
    vertices_[3] = transform * Vector3(-_near.x_, _near.y_, _near.z_);
    vertices_[4] = transform * _far;
    vertices_[5] = transform * Vector3(_far.x_, -_far.y_, _far.z_);
    vertices_[6] = transform * Vector3(-_far.x_, -_far.y_, _far.z_);
    vertices_[7] = transform * Vector3(-_far.x_, _far.y_, _far.z_);
    UpdatePlanes();
}

Frustum Frustum::Transformed(const Matrix4& transform) const
{
    Frustum transformed;

    // Transform all vertices
    for (unsigned i = 0; i < 8; ++i)
        transformed.vertices_[i] = transform * vertices_[i];

    transformed.UpdatePlanes();

    return transformed;
}

Intersection Frustum::IsInside(const BoundingBox& bbox)
{
    // https://www.gamedev.net/resources/_/technical/general-programming/frustum-culling-r4613
    bool inside = true;
    // Test all 6 frustum planes
    for (unsigned i = 0; i < CountPlanes; i++)
    {
        // Pick closest point to plane and check if it is behind the plane
        // if yes - object outside frustum
        const Plane& plane = planes_[i];
        const float d =
            std::max<float>(bbox.min_.x_ * plane.normal_.x_, bbox.max_.x_ * plane.normal_.x_) +
            std::max<float>(bbox.min_.y_ * plane.normal_.y_, bbox.max_.y_ * plane.normal_.y_) +
            std::max<float>(bbox.min_.z_ * plane.normal_.z_, bbox.max_.z_ * plane.normal_.z_) +
            plane.d_;
        inside &= d > 0;
        //return false; //with flag works faster
    }
    return inside ? INSIDE : OUTSIDE;
}

Intersection Frustum::IsInside(const Sphere& sphere) const
{
    bool allInside = true;
    for (const auto& plane : planes_)
    {
        float dist = plane.Distance(sphere.center_);
        if (dist < -sphere.radius_)
            return OUTSIDE;
        else if (dist < sphere.radius_)
            allInside = false;
    }

    return allInside ? INSIDE : INTERSECTS;
}

Shape Frustum::GetShape() const
{
    Shape s;
    for (unsigned i = 0; i < 8; ++i)
        s.vertexData_.push_back(vertices_[i]);
    s.vertexCount_ = 8;

/*
        7------------4
      / |          / |
    3------------0   |
    |   |        |   |
    |   6------------5
    | /          | /
    2------------1
*/
    // Near
    s.AddTriangle(0, 1, 2);
    s.AddTriangle(2, 3, 0);
    // Left
    s.AddTriangle(2, 3, 6);
    s.AddTriangle(6, 7, 3);
    // Right
    s.AddTriangle(1, 0, 4);
    s.AddTriangle(4, 5, 1);
    // Up
    s.AddTriangle(0, 4, 7);
    s.AddTriangle(7, 3, 0);
    // Down
    s.AddTriangle(6, 5, 1);
    s.AddTriangle(1, 2, 6);
    // Far
    s.AddTriangle(5, 6, 7);
    s.AddTriangle(7, 4, 5);

    return s;
}

}
