/**
 * Copyright 2020 Stefan Ascher
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

#include "TriangleMesh.h"
#include "Gjk.h"
#include "ConvexHull.h"
#include "Hull.h"
#include "Vector3.h"
#include "Sphere.h"
#include "HeightMap.h"

namespace Math {

TriangleMesh::TriangleMesh(TriangleMesh&& other) noexcept
{
    vertexData_ = std::move(other.vertexData_);
    vertexCount_ = other.vertexCount_;
    indexData_ = std::move(other.indexData_);
    indexCount_ = other.indexCount_;
    matrix_ = std::move(other.matrix_);
    boundingBox_ = std::move(other.boundingBox_);
}

TriangleMesh::TriangleMesh(const Shape& other) :
    Shape(other)
{
    boundingBox_.Merge(other.vertexData_.data(), other.vertexData_.size());
}

Intersection TriangleMesh::IsInside(const Vector3& point) const
{
    Shape shape2(point);
    if (Gjk::StaticIntersects(*this, shape2))
        return Intersection::Inside;
    return Intersection::Outside;
}

TriangleMesh TriangleMesh::Transformed(const Matrix4& transform) const
{
    return TriangleMesh(*this, transform);
}

bool TriangleMesh::Collides(const Sphere& b2, const Vector3&, Vector3&) const
{
    return Gjk::StaticIntersects(*this, b2.GetShape());
}

bool TriangleMesh::Collides(const BoundingBox& b2, const Vector3&, Vector3&) const
{
    return b2.IsInside(*this) != Intersection::Outside;
}

bool TriangleMesh::Collides(const ConvexHull& b2, const Vector3&, Vector3&) const
{
    return Gjk::StaticIntersects(*this, b2.GetShape());
}

bool TriangleMesh::Collides(const TriangleMesh& b2, const Vector3&, Vector3&) const
{
    return Gjk::StaticIntersects(*this, b2.GetShape());
}

bool TriangleMesh::Collides(const HeightMap& b2, const Vector3&, Vector3& move) const
{
    const Vector3 pBottom = GetFarsetPointInDirection(Math::Vector3::Down);
    const float y = b2.GetHeight(pBottom);
    if (pBottom.y_ < y)
    {
        move.y_ = pBottom.y_ - y;
        return true;
    }
    return false;
}

}