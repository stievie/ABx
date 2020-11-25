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

#include "ConvexHull.h"
#include "Hull.h"
#include "Vector3.h"
#include "Gjk.h"
#include "Sphere.h"
#include "HeightMap.h"
#include "TriangleMesh.h"
#include <cstring>

namespace Math {

ConvexHull::ConvexHull(const ea::vector<Vector3>& vertices) :
    Shape()
{
    BuildHull(vertices);
}

Intersection ConvexHull::IsInside(const Vector3& point) const
{
    Shape shape2(point);
    if (Gjk::StaticIntersects(*this, shape2))
        return Intersection::Inside;
    return Intersection::Outside;
}

ConvexHull ConvexHull::Transformed(const Matrix4& transform) const
{
    ConvexHull result(*this);
    result.matrix_ = transform;
    return result;
}

bool ConvexHull::Collides(const Sphere& b2, const Vector3&, Vector3&) const
{
    const Shape s = b2.GetShape();

    if (Gjk::StaticIntersects(*this, s))
    {
        return true;
    }
    return false;
}

bool ConvexHull::Collides(const BoundingBox& b2, const Vector3&, Vector3&) const
{
    const Shape s = b2.GetShape();

    if (Gjk::StaticIntersects(*this, s))
    {
        return true;
    }
    return false;
}

bool ConvexHull::Collides(const ConvexHull& b2, const Vector3&, Vector3&) const
{
    if (Gjk::StaticIntersects(*this, b2))
        return true;
    return false;
}

bool ConvexHull::Collides(const TriangleMesh& b2, const Vector3&, Vector3&) const
{
    if (Gjk::StaticIntersects(*this, b2))
        return true;
    return false;
}

bool ConvexHull::Collides(const HeightMap& b2, const Vector3&, Vector3& move) const
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

void ConvexHull::BuildHull(const ea::vector<Vector3>& vertices)
{
    if (vertices.size())
    {
        // Build the convex hull from the raw geometry
        StanHull::HullDesc desc;
        desc.SetHullFlag(StanHull::QF_TRIANGLES);
        desc.mVcount = static_cast<unsigned>(vertices.size());
        desc.mVertices = &vertices[0].x_;
        desc.mVertexStride = 3 * sizeof(float);
        desc.mSkinWidth = 0.0f;

        StanHull::HullLibrary lib;
        StanHull::HullResult result;
        lib.CreateConvexHull(desc, result);

        vertexCount_ = result.mNumOutputVertices;
        vertexData_.resize(vertexCount_);

        // Copy vertex data & index data
        std::memcpy(vertexData_.data(), result.mOutputVertices, vertexCount_ * (sizeof(float) * 3));

        indexCount_ = result.mNumIndices;
        indexData_.reserve(indexCount_);
        for (size_t i = 0; i < indexCount_; ++i)
        {
            indexData_.push_back(result.mIndices[i]);
        }

        lib.ReleaseResult(result);

        boundingBox_.Merge(&vertexData_[0], vertexCount_);
    }
    else
    {
        vertexData_.clear();
        vertexCount_ = 0;
        indexData_.clear();
        indexCount_ = 0;
    }
}

}
