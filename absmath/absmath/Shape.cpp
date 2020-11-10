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

#include "Shape.h"
#include "VectorMath.h"

namespace Math {

void Shape::Reset()
{
    vertexData_.clear();
    vertexCount_ = 0;
    indexData_.clear();
    indexCount_ = 0;
}

void Shape::AddTriangle(unsigned i1, unsigned i2, unsigned i3)
{
    indexData_.push_back(i1);
    indexData_.push_back(i2);
    indexData_.push_back(i3);
    indexCount_ += 3;
}

bool Shape::IsFacingOutside(const ea::array<Vector3, 3>& triangle) const
{
    // We only support convex shapes so calculate the normal and check if
    // it points outside.
    const Vector3 normal = GetTriangleNormal(triangle[0], triangle[1], triangle[2]);
    const Vector3 center = Center();
    const float r = normal.DotProduct(center);
    return r < 0.0f;
}

Vector3 Shape::Center() const
{
    Vector3 result;
    for (size_t i = 0; i < GetCount(); ++i)
        result += GetVertex(i);
    result /= static_cast<float>(GetCount());
    return result;
}

Vector3 Shape::GetClosestPointOnTriangle(const ea::array<Vector3, 3>& tri, const Vector3& pos) const
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

Vector3 Shape::GetClosestPointOnTriangle(size_t i, const Vector3& pos) const
{
    if (GetTriangleCount() == 0)
        return {};
    return GetClosestPointOnTriangle(GetTriangle(i), pos);
}

float Shape::GetDistanceToTriangle(const ea::array<Vector3, 3>& tri, const Vector3& pos) const
{
    const Vector3 normal = GetTriangleNormal(tri[0], tri[1], tri[2]);
    const Vector3 v = pos - tri[0];
    const float dot = v.DotProduct(normal);
    const Vector3 s = v - (normal * dot);
    return s.Length();
}

float Shape::GetDistanceToTriangle(size_t i, const Vector3& pos) const
{
    if (GetTriangleCount() == 0)
        return std::numeric_limits<float>::max();
    return GetDistanceToTriangle(GetTriangle(i), pos);
}

size_t Shape::GetClosestTriangleIndex(const Vector3& pos) const
{
    float minDist = std::numeric_limits<float>::max();
    size_t best = 0;
    const size_t count = GetTriangleCount();
    for (size_t i = 0; i < count; ++i)
    {
        float dist = GetDistanceToTriangle(i, pos);
        if (dist < minDist)
        {
            minDist = dist;
            best = i;
        }
    }
    return best;
}

Vector3 Shape::GetFarsetPointInDirection(const Vector3& direction) const
{
    size_t best = 0;
    float farest = GetVertex(0).DotProduct(direction);

    const size_t count = GetCount();
    for (size_t i = 1; i < count; ++i)
    {
        float d = GetVertex(i).DotProduct(direction);
        if (farest < d)
        {
            best = i;
            farest = d;
        }
    }
    return GetVertex(best);
}

Shape Shape::Transformed(const Matrix4& transformation) const
{
    Shape result(*this);
    result.matrix_ = transformation;
    return result;
}

float Shape::GetMinHeight() const
{
    float result = std::numeric_limits<float>::max();
    for (const auto& v : vertexData_)
    {
        if (v.y_ < result)
            result = v.y_;
    }
    return result;
}

float Shape::GetMaxHeight() const
{
    float result = std::numeric_limits<float>::min();
    for (const auto& v : vertexData_)
    {
        if (v.y_ > result)
            result = v.y_;
    }
    return result;
}

}
