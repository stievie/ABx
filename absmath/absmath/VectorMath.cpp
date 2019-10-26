#include "stdafx.h"
#include "VectorMath.h"
#include "MathDefs.h"
#include "MathUtils.h"

namespace Math {

bool IsPointInTriangle(const Vector3& point, const Vector3& a, const Vector3& b, const Vector3& c)
{
    const Vector3 v1 = (point - a).Normal();
    const Vector3 v2 = (point - b).Normal();
    const Vector3 v3 = (point - c).Normal();

    float angles = 0.0f;
    angles += acos(v1.DotProduct(v2));
    angles += acos(v2.DotProduct(v3));
    angles += acos(v3.DotProduct(v1));

    if (fabsf(angles - 2 * M_PIF) <= 0.005f)
        return true;
    return false;
}

bool IsPointInSphere(const Vector3& point, const Sphere& s)
{
    float d = (point - s.center_).Length();
    if (d <= s.radius_)
        return true;
    return false;
}

Vector3 GetClosestPointOnLine(const Vector3& a, const Vector3& b, const Vector3& p)
{
    const Vector3 c = p - a;
    Vector3 V = b - a;
    float d = V.Length();

    V.Normalize();
    float t = V.DotProduct(c);
    if (t < 0.0f)
        return a;
    if (t > d)
        return b;

    V *= t;

    return a + V;
}

Vector3 GetClosestPointOnTriangle(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p)
{
    Vector3 Rab = GetClosestPointOnLine(a, b, p);
    Vector3 Rbc = GetClosestPointOnLine(b, c, p);
    Vector3 Rca = GetClosestPointOnLine(c, a, p);

    float dAB = (p - Rab).Length();
    float dBC = (p - Rbc).Length();
    float dCA = (p - Rca).Length();

    Vector3 result = Rab;

    float m = dAB;
    if (dBC < m)
    {
        m = dBC;
        result = Rbc;
    }
    if (dCA < m)
        result = Rca;

    return result;
}

float IntersectsRaySphere(const Ray& ray, const Sphere& sphere)
{
    const Vector3 Q = sphere.center_ - ray.origin_;
    float c = Q.Length();
    float v = Q.DotProduct(ray.direction_);
    float d = sphere.radius_*sphere.radius_ - (c*c - v*v);
    if (d < 0.0f)
        // No intersection
        return -1.0f;

    return v - sqrtf(d);
}

float IntersectsRayPlane(const Ray& ray, const Vector3& planeOrigin, const Vector3& planeNormal)
{
    float d = -planeNormal.DotProduct(planeOrigin);
    float numer = planeNormal.DotProduct(ray.origin_) + d;
    float denom = planeNormal.DotProduct(ray.direction_);
    if (Equals(denom, 0.0f))
        return -1.0f;
    return -(numer / denom);
}

PointClass GetPointClass(const Vector3& point, const Vector3& origin, const Vector3& normal)
{
    const Vector3 dir = origin - point;
    float d = dir.DotProduct(normal);
    if (d < -0.001f)
        return PointClass::PlaneFront;
    if (d > 0.001f)
        return PointClass::PlaneBack;

    return PointClass::OnPlane;
}

}