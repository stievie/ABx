#pragma once

#include "BoundingBox.h"
#include "Shape.h"

namespace Math {

class Vector3;
class HeightMap;

class ConvexHull : public Shape
{
private:
    void BuildHull(const std::vector<Vector3>& vertices);
public:
    ConvexHull() = default;
    ConvexHull(const ConvexHull& other) :
        Shape(other),
        boundingBox_(other.boundingBox_)
    {}
    ConvexHull(ConvexHull&& other) noexcept :
        Shape(other),
        boundingBox_(std::move(other.boundingBox_))
    {}
    explicit ConvexHull(const std::vector<Vector3>& vertices);
    ~ConvexHull() = default;

    ConvexHull& operator= (const ConvexHull& other)
    {
        if (this != &other)
        {
            vertexData_ = other.vertexData_;
            vertexCount_ = other.vertexCount_;
            indexData_ = other.indexData_;
            indexCount_ = other.indexCount_;
        }
        return *this;
    }
    ConvexHull& operator= (ConvexHull&& other) noexcept
    {
        vertexData_ = std::move(other.vertexData_);
        vertexCount_ = other.vertexCount_;
        indexData_ = std::move(other.indexData_);
        indexCount_ = other.indexCount_;
        boundingBox_ = std::move(other.boundingBox_);
        return *this;
    }

    BoundingBox GetBoundingBox() const
    {
        return boundingBox_;
    }
    Intersection IsInside(const Vector3& point) const;
    ConvexHull Transformed(const Matrix4& transform) const;
    bool Collides(const Sphere& b2, const Vector3& velocity, Vector3& move) const;
    bool Collides(const BoundingBox& b2, const Vector3& velocity, Vector3& move) const;
    bool Collides(const ConvexHull& b2, const Vector3& velocity, Vector3& move) const;
    bool Collides(const HeightMap& b2, const Vector3& velocity, Vector3& move) const;

    Shape GetShape() const { return *this; }

    BoundingBox boundingBox_;
};

}
