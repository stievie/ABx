#pragma once

#include "Point.h"
#include "Vector3.h"
#include "BoundingBox.h"
#include "Point.h"
#include "Matrix4.h"

namespace Math {

class HeightMap
{
private:
public:
    HeightMap();
    HeightMap(const std::vector<float>& data, const Point<int>& size);
    ~HeightMap() = default;

    void ProcessData();

    float GetRawHeight(int x, int z) const;
    Vector3 GetRawNormal(int x, int z) const;
    /// Return height at world coordinates.
    float GetHeight(const Vector3& world, const Matrix4& matrix = Matrix4::Identity) const;
    Vector3 GetNormal(const Vector3& world, const Matrix4& matrix) const;

    BoundingBox GetBoundingBox() const
    {
        return boundingBox_;
    }

    Point<int> WorldToHeightmap(const Vector3& world, const Matrix4& matrix);
    Vector3 HeightmapToWorld(const Point<int>& pixel, const Matrix4& matrix);

    /// Vertex and height spacing.
    Vector3 spacing_;
    std::vector<float> heightData_;
    float minHeight_;
    float maxHeight_;
    Point<int> numVertices_;
    BoundingBox boundingBox_;
};

}

