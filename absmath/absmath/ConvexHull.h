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
    explicit ConvexHull(const std::vector<Vector3>& vertices);
    ~ConvexHull() = default;

    BoundingBox GetBoundingBox() const
    {
        return boundingBox_;
    }
    Intersection IsInside(const Vector3& point) const;
    ConvexHull Transformed(const Matrix4& transform) const;
    bool Collides(const Sphere& b2, Vector3& move) const;
    bool Collides(const BoundingBox& b2, Vector3& move) const;
    bool Collides(const ConvexHull& b2, Vector3& move) const;
    bool Collides(const HeightMap& b2, Vector3& move) const;

    Shape GetShape() const { return *this; }

    BoundingBox boundingBox_;
};

}
