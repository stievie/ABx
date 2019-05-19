#include "stdafx.h"
#include "Plane.h"

namespace Math {

void Plane::Define(const Vector3& v0, const Vector3& v1, const Vector3& v2)
{
    const Vector3 dist1 = v1 - v0;
    const Vector3 dist2 = v2 - v0;

    Define(dist1.CrossProduct(dist2), v0);
}

/// Define from a normal vector and a point on the plane.
void Plane::Define(const Vector3& normal, const Vector3& point)
{
    normal_ = normal.Normal();
    d_ = -normal_.DotProduct(point);
}

}
