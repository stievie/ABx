#pragma once

#include "Point.h"
#include "Vector3.h"
#include "BoundingBox.h"
#include "Point.h"
#include "Matrix4.h"

namespace Math {

class Shape;

class HeightMap
{
public:
    HeightMap();
    HeightMap(const std::vector<float>& data, const Point<int>& size);
    ~HeightMap() = default;

    void ProcessData();

    float GetRawHeight(int x, int z) const;
    Vector3 GetRawNormal(int x, int z) const;
    /// Return height at world coordinates.
    float GetHeight(const Vector3& world) const;
    Vector3 GetNormal(const Vector3& world) const;

    BoundingBox GetBoundingBox() const
    {
        return boundingBox_;
    }
    const HeightMap& Transformed(const Matrix4&) const
    {
        return *this;
    }

    bool Collides(const Sphere& b2, const Vector3& velocity, Vector3& move) const;
    bool Collides(const BoundingBox& b2, const Vector3& velocity, Vector3& move) const;
    bool Collides(const ConvexHull& b2, const Vector3& velocity, Vector3& move) const;
    bool Collides(const HeightMap&, const Vector3&, Vector3&) const;

    Shape GetShape() const;

    Point<int> WorldToHeightmap(const Vector3& world);
    Vector3 HeightmapToWorld(const Point<int>& pixel);
    int GetWidth() const
    {
        return numVertices_.x_;
    }
    int GetHeight() const
    {
        return numVertices_.y_;
    }

    /// Vertex and height spacing.
    Vector3 spacing_;
    float minHeight_;
    float maxHeight_;
    Point<int> numVertices_;
    std::vector<float> heightData_;
    BoundingBox boundingBox_;
    /// Transformation matrix
    Matrix4 matrix_ = Matrix4::Identity;
};

}

