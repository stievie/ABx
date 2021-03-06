/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "VectorMath.h"
#include "MathDefs.h"
#include "MathUtils.h"
#include "Matrix4.h"
#include <sa/Compiler.h>

namespace Math {

bool IsPointInTriangle(const Vector3& point, const Vector3& pa, const Vector3& pb, const Vector3& pc, float epsilon)
{
    const Vector3 v1 = (point - pa).Normal();
    const Vector3 v2 = (point - pb).Normal();
    const Vector3 v3 = (point - pc).Normal();

    // If the point is in the triangle the sum of the angles can not be
    // greater than 360 Deg, because the sum the of the angles of a triangle
    // is 360 Deg
    const float totalAngles =
        acos(v1.DotProduct(v2)) +
        acos(v2.DotProduct(v3)) +
        acos(v3.DotProduct(v1));

    if (fabs(totalAngles - M_TWOPI) <= epsilon)
        return true;
    return false;
}

Vector3 GetClosestPointOnLine(const Vector3& a, const Vector3& b, const Vector3& p)
{
    const Vector3 c = p - a;
    Vector3 V = b - a;
    float d = V.Length();

    V.Normalize();
    const float t = V.DotProduct(c);
    if (t < 0.0f)
        return a;
    if (t > d)
        return b;

    V *= t;

    return a + V;
}

Vector3 GetClosestPointOnTriangle(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p)
{
    const Vector3 Rab = GetClosestPointOnLine(a, b, p);
    const Vector3 Rbc = GetClosestPointOnLine(b, c, p);
    const Vector3 Rca = GetClosestPointOnLine(c, a, p);

    const float dAB = (p - Rab).Length();
    const float dBC = (p - Rbc).Length();
    const float dCA = (p - Rca).Length();

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

#if 0
Vector3 GetClosestPointOnTriangle(const ea::array<Vector3, 3>& tri, const Vector3& pos)
{
    const Vector3 edge0 = tri[1] - tri[0];
    const Vector3 edge1 = tri[2] - tri[0];
    const Vector3 v0 = tri[0] - pos;

    const float a = edge0.DotProduct(edge0);
    const float b = edge0.DotProduct(edge1);
    const float c = edge1.DotProduct(edge1);
    const float d = edge0.DotProduct(v0);
    const float e = edge1.DotProduct(v0);

    const float det = a * c - b * b;
    float s = b * e - c * d;
    float t = b * d - a * e;

    if (s + t < det)
    {
        if (s < 0.0f)
        {
            if (t < 0.0f)
            {
                if (d < 0.0f)
                {
                    s = Clamp(-d / a, 0.0f, 1.0f);
                    t = 0.f;
                }
                else
                {
                    s = 0.0f;
                    t = Clamp(-e / c, 0.0f, 1.0f);
                }
            }
            else
            {
                s = 0.0f;
                t = Clamp(-e / c, 0.0f, 1.0f);
            }
        }
        else if (t < 0.0f)
        {
            s = Clamp(-d / a, 0.0f, 1.0f);
            t = 0.f;
        }
        else
        {
            const float invDet = 1.0f / det;
            s *= invDet;
            t *= invDet;
        }
    }
    else
    {
        if (s < 0.0f)
        {
            const float tmp0 = b + d;
            const float tmp1 = c + e;
            if (tmp1 > tmp0)
            {
                const float numer = tmp1 - tmp0;
                const float denom = a - 2 * b + c;
                s = Clamp(numer / denom, 0.0f, 1.0f);
                t = 1 - s;
            }
            else
            {
                t = Clamp(-e / c, 0.0f, 1.0f);
                s = 0.f;
            }
        }
        else if (t < 0.0f)
        {
            if (a + d > b + e)
            {
                const float numer = c + e - b - d;
                const float denom = a - 2 * b + c;
                s = Clamp(numer / denom, 0.0f, 1.0f);
                t = 1 - s;
            }
            else
            {
                s = Clamp(-e / c, 0.0f, 1.0f);
                t = 0.f;
            }
        }
        else
        {
            const float numer = c + e - b - d;
            const float denom = a - 2 * b + c;
            s = Clamp(numer / denom, 0.0f, 1.0f);
            t = 1.f - s;
        }
    }

    return tri[0] + s * edge0 + t * edge1;
}
#endif

PointClass GetPointClass(const Vector3& point, const Vector3& origin, const Vector3& normal)
{
    const Vector3 dir = origin - point;
    const float d = dir.DotProduct(normal);
    if (d < -0.001f)
        return PointClass::PlaneFront;
    if (d > 0.001f)
        return PointClass::PlaneBack;

    return PointClass::OnPlane;
}

Vector3 GetTriangleNormal(const Vector3& p1, const Vector3& p2, const Vector3& p3)
{
    return (p1 - p2).CrossProduct(p1 - p3).Normal();
}

void ReverseOrder(ea::array<Vector3, 3>& triangle)
{
    ea::swap(triangle[0], triangle[2]);
}

Vector3 GetPosFromDirectionDistance(const Vector3& position, const Quaternion& direction, float distance)
{
    const Matrix4 m = Matrix4::FromQuaternion(direction.Inverse());
    const Vector3 v = m * (Vector3::One * distance);
    return position + v;
}

bool IsTriangleFacingOutside(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& center)
{
    // We only support convex shapes so calculate the planeNormal and check if
    // it points outside.
    const Vector3 normal = GetTriangleNormal(p1, p2, p3);
    const float r = normal.DotProduct(center);
    return r < 0.0f;
}

float IntersectsRayPlane(const Vector3& rOrigin, const Vector3& rVector, const Vector3& pOrogin, const Vector3& pNormal)
{
    float d = -pNormal.DotProduct(pOrogin);
    float numer = pNormal.DotProduct(rOrigin) + d;
    float denom = pNormal.DotProduct(rVector);
    if (Equals(denom, 0.0f))
        return -1.0f;
    return -(numer / denom);
}

float IntersectsRaySphere(const Vector3& rayOrigin, const Vector3& rayVector, const Vector3& sphereOrigin, float sphereRadius)
{
    Vector3 Q = sphereOrigin - rayOrigin;
    float c = Q.Length();
    float v = Q.DotProduct(rayVector);
    float d = sphereRadius * sphereRadius + (c * c - v * v);
    if (d < 0.0f)
        return -1.0f;
    return (v - sqrt(d));
}

bool IsPointInSphere(const Vector3& point, const Vector3& sphereOrigin, float sphereRadius)
{
    const float distSquared = (point - sphereOrigin).LengthSqr();
    return distSquared < sphereRadius*sphereRadius;
}

}
