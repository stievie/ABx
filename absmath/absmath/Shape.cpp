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
    return IsTriangleFacingOutside(triangle[0], triangle[1], triangle[2], Center());
}

Vector3 Shape::Center() const
{
    Vector3 result;
    for (size_t i = 0; i < GetCount(); ++i)
        result += GetVertex(i);
    result /= static_cast<float>(GetCount());
    return result;
}

Vector3 Shape::GetClosestPointOnTriangle(size_t i, const Vector3& pos) const
{
    if (GetTriangleCount() == 0)
        return {};
    return Math::GetClosestPointOnTriangle(GetTriangle(i), pos);
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
        return M_INFINITE;
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

#if 0
Shape Shape::Transformed(const Matrix4& transformation) const
{
    Shape result(*this);
    result.matrix_ = transformation;
    return result;
}
#endif

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
