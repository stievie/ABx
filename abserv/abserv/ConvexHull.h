#pragma once

#include "BoundingBox.h"

namespace Math {

class Vector3;

class ConvexHull
{
private:
    unsigned vertexCount_;
    unsigned indexCount_;
    std::vector<Vector3> vertexData_;
    std::vector<unsigned> indexData_;
    BoundingBox boundingBox_;
    void BuildHull(const std::vector<Vector3>& vertices);
public:
    explicit ConvexHull(const std::vector<Vector3>& vertices);
    ~ConvexHull();

    BoundingBox GetBoundingBox() const;
};

}
