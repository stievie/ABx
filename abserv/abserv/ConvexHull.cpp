#include "stdafx.h"
#include "ConvexHull.h"
#include "Hull.h"
#include "Vector3.h"

namespace Math {

ConvexHull::ConvexHull(const std::vector<Vector3>& vertices)
{
    BuildHull(vertices);
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
