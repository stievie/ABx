#pragma once

#include "Point.h"
#include "Vector3.h"
#include "BoundingBox.h"

namespace Math {

class HeightMap
{
private:
public:
    HeightMap();
    HeightMap(const std::vector<float>& data, const Point<int>& size);
    ~HeightMap();

    float GetRawHeight(int x, int z) const;
    Vector3 GetRawNormal(int x, int z) const;
    BoundingBox GetBoundingBox() const
    {
        return boundingBox_;
    }

    /// Vertex and height spacing.
    Vector3 spacing_;
    std::vector<float> heightData_;
    float minHeight_;
    float maxHeight_;
    Point<int> numVertices_;
    BoundingBox boundingBox_;
};

}

