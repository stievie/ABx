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

namespace Math {

class Plane
{
public:
    constexpr Plane() noexcept :
        d_(0.0f)
    { }
    constexpr Plane(const Vector3& normal, float distance) noexcept :
        normal_(normal),
        d_(distance)
    { }
    Plane(const Vector3& normal, const Vector3& point)
    {
        Define(normal, point);
    }
    Plane(const Vector3& p1, const Vector3& p2, const Vector3& p3)
    {
        Define(p1, p2, p3);
    }
    ~Plane() = default;

    /// Define from 3 vertices.
    void Define(const Vector3& v0, const Vector3& v1, const Vector3& v2);
    /// Define from a normal vector and a point on the plane.
    void Define(const Vector3& normal, const Vector3& point);

    float Distance(const Vector3& point) const { return normal_.DotProduct(point) + d_; }
    bool IsFrontFacingTo(const Vector3& direction) const { return normal_.DotProduct(direction) <= 0.0f; }

    Vector3 normal_;
    float d_;
};

}
