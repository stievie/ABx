#pragma once

#include "Vector3.h"

namespace Math {

class Sphere;

enum class PointClass
{
    PlaneFront,
    PlaneBack,
    OnPlane
};

// Check if a point is inside a triangle
bool IsPointInTriangle(const Vector3& point, const Vector3& pa, const Vector3& pb, const Vector3& pc);
// Get closest point on line segment
Vector3 GetClosestPointOnLine(const Vector3& a, const Vector3& b, const Vector3& p);
Vector3 GetClosestPointOnTriangle(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p);
PointClass GetPointClass(const Vector3& point, const Vector3& origin, const Vector3& normal);

}