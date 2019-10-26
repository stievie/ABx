#pragma once

#include "Vector3.h"
#include "Ray.h"
#include "Sphere.h"

namespace Math {

enum class PointClass
{
    PlaneFront,
    PlaneBack,
    OnPlane
};

// Check if a point is inside a triangle
bool IsPointInTriangle(const Vector3& point, const Vector3& a, const Vector3& b, const Vector3& c);
bool IsPointInSphere(const Vector3& point, const Sphere& s);
// Get closest point on line segment
Vector3 GetClosestPointOnLine(const Vector3& a, const Vector3& b, const Vector3& p);
Vector3 GetClosestPointOnTriangle(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p);
// Return distance to sphere or -1
float IntersectsRaySphere(const Ray& ray, const Sphere& sphere);
// Return distance to plane or -1
float IntersectsRayPlane(const Ray& ray, const Vector3& planeOrigin, const Vector3& planeNormal);
PointClass GetPointClass(const Vector3& point, const Vector3& origin, const Vector3& normal);

inline Vector3 WEdge(const Vector3& v1, const Vector3& v2)
{
    Vector3 result;
    result.x_ = (v1.y_ * v2.z_) - (v2.y_ * v1.z_);
    result.y_ = (v1.z_ * v2.x_) - (v2.z_ * v1.x_);
    result.z_ = (v1.x_ * v2.y_) - (v2.x_ * v1.y_);

    return result;
}

}