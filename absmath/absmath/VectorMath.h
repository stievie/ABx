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

#pragma once

#include "Vector3.h"
#include "Quaternion.h"

namespace Math {

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
/// Get face normal vector.
Vector3 GetTriangleNormal(const Vector3& p1, const Vector3& p2, const Vector3& p3);
/// Make CCW -> CW and vice versa
void ReverseOrder(std::array<Vector3, 3>& triangle);
Vector3 GetPosFromDirectionDistance(const Vector3& position, const Quaternion& direction, float distance);

}
