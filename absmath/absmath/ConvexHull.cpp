#include "stdafx.h"
#include "ConvexHull.h"
#include "Hull.h"
#include "Vector3.h"
#include "Gjk.h"
#include "Sphere.h"
#include "HeightMap.h"

namespace Math {

ConvexHull::ConvexHull(const std::vector<Vector3>& vertices) :
    Shape()
{
    BuildHull(vertices);
}

Intersection ConvexHull::IsInside(const Vector3& point) const
{
    Shape shape2(point);
    if (Gjk::StaticIntersects(*this, shape2))
        return INSIDE;
    return OUTSIDE;
}

ConvexHull ConvexHull::Transformed(const Matrix4& transform) const
{
    ConvexHull result(*this);
    result.matrix_ = transform;
    return result;
}

bool ConvexHull::Collides(const Sphere& b2, Vector3&) const
{
    const Shape s = b2.GetShape();

    if (Gjk::StaticIntersects(*this, s))
    {
        return true;
    }
    return false;
}

bool ConvexHull::Collides(const BoundingBox& b2, Vector3&) const
{
    const Shape s = b2.GetShape();

    if (Gjk::StaticIntersects(*this, s))
    {
        return true;
    }
    return false;
}

bool ConvexHull::Collides(const ConvexHull& b2, Vector3&) const
{
    if (Gjk::StaticIntersects(*this, b2))
        return true;
    return false;
}

bool ConvexHull::Collides(const HeightMap& b2, Vector3& move) const
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

void ConvexHull::BuildHull(const std::vector<Vector3>& vertices)
{
    if (vertices.size())
    {
        // Build the convex hull from the raw geometry
        StanHull::HullDesc desc;
        desc.SetHullFlag(StanHull::QF_TRIANGLES);
        desc.mVcount = (unsigned)vertices.size();
        desc.mVertices = vertices[0].Data();
        desc.mVertexStride = 3 * sizeof(float);
        desc.mSkinWidth = 0.0f;

        StanHull::HullLibrary lib;
        StanHull::HullResult result;
        lib.CreateConvexHull(desc, result);

        vertexCount_ = result.mNumOutputVertices;
        vertexData_.resize(vertexCount_);

        indexCount_ = result.mNumIndices;
        indexData_.resize(indexCount_);

        // Copy vertex data & index data
        memcpy(vertexData_.data(), result.mOutputVertices, vertexCount_ * sizeof(Vector3));
        memcpy(indexData_.data(), result.mIndices, indexCount_ * sizeof(unsigned));

        lib.ReleaseResult(result);

        boundingBox_.Merge(&vertexData_[0], vertexCount_);
    }
    else
    {
        vertexCount_ = 0;
        indexCount_ = 0;
    }
}

}
