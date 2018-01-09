#pragma once

#include "BoundingBox.h"

namespace Math {

class Vector3;

class ConvexHull
{
private:
    BoundingBox boundingBox_;
    void BuildHull(const std::vector<Vector3>& vertices);
public:
    ConvexHull() = default;
    explicit ConvexHull(const std::vector<Vector3>& vertices);
    ~ConvexHull() = default;

    BoundingBox GetBoundingBox() const;

    unsigned vertexCount_;
    unsigned indexCount_;
    std::vector<Vector3> vertexData_;
    std::vector<unsigned> indexData_;
};

}
