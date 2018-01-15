#pragma once

#include "BoundingBox.h"

namespace Math {

class Vector3;

class ConvexHull
{
private:
    void BuildHull(const std::vector<Vector3>& vertices);
public:
    ConvexHull() = default;
    explicit ConvexHull(const std::vector<Vector3>& vertices);
    ~ConvexHull() = default;

    BoundingBox GetBoundingBox() const
    {
        return boundingBox_;
    }
    Intersection IsInside(const Vector3& point) const;

    BoundingBox boundingBox_;
    unsigned vertexCount_;
    unsigned indexCount_;
    std::vector<Vector3> vertexData_;
    std::vector<unsigned> indexData_;
};

}
