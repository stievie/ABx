#pragma once

#include "Vector3.h"
#include "Plane.h"
#include "Matrix4.h"
#include "MathDefs.h"

namespace Math {

class BoundingBox;

class Frustum
{
public:
    enum FrustumPlanes : uint8_t
    {
        PlaneNear = 0,
        PlaneLeft,
        PlaneRight,
        PlaneUp,
        PlaneDown,
        PlaneFar,
        CountPlanes
    };
private:
    void UpdatePlanes();
public:
    Frustum() noexcept
    { };
    Frustum(const Frustum& other);

    ~Frustum() = default;

    void Define(float fov, float aspectRatio, float zoom,
        float nearZ, float farZ,
        const Matrix4& transform = Matrix4::Identity);
    void Define(const Vector3& _near, const Vector3& _far,
        const Matrix4& transform = Matrix4::Identity);

    Frustum Transformed(const Matrix4& transform) const;

    Intersection Intersects(const BoundingBox& bbox);

    Vector3 vertices_[8];
    Plane planes_[FrustumPlanes::CountPlanes];
};

}
