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
#include <sa/Iteration.h>

namespace Math {

enum class PointClass
{
    PlaneFront,
    PlaneBack,
    OnPlane
};

// Check if a point is inside a triangle
bool IsPointInTriangle(const Vector3& point, const Vector3& pa, const Vector3& pb, const Vector3& pc, float epsilon = 0.005f);
// Get closest point on line segment
Vector3 GetClosestPointOnLine(const Vector3& a, const Vector3& b, const Vector3& p);
Vector3 GetClosestPointOnTriangle(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p);
PointClass GetPointClass(const Vector3& point, const Vector3& origin, const Vector3& normal);
/// Get face normal vector.
Vector3 GetTriangleNormal(const Vector3& p1, const Vector3& p2, const Vector3& p3);
/// Make CCW -> CW and vice versa
void ReverseOrder(std::array<Vector3, 3>& triangle);
Vector3 GetPosFromDirectionDistance(const Vector3& position, const Quaternion& direction, float distance);
bool IsTriangleFacingOutside(const Vector3& p1, const Vector3& p2, const Vector3& p3, const Vector3& center);

template<typename Callback>
inline void GetTriangleIndices(int width, int height, Callback&& callback)
{
    // Create index data in clockwise order
    for (int y = 0; y < height - 1; ++y)
    {
        for (int x = 0; x < width - 1; ++x)
        {
            /*
                x+1,y
        x,y +----+----+
            | 1 /|(3)/|
            |  / |  / |
            | /  | /  |
            |/ 2 |/(4)|
      x,y+1 +----+----+
              x+1,y+1
            */
            {
                // First triangle
                int i1 = (y + 1) * width + x;
                int i2 = y * width + x;
                int i3 = (y * width) + x + 1;
                if (callback(i3, i2, i1) == Iteration::Break)
                    return;
            }

            {
                // Second triangle
                int i1 = y * width + x + 1;
                int i2 = (y + 1) * width + (x + 1);
                int i3 = (y + 1) * width + x;
                if (callback(i3, i2, i1) == Iteration::Break)
                    return;
            }
        }
    }
}

}
