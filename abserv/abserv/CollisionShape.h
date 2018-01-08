#pragma once

namespace Math {

class Vector3;
class ConvexHull;
class BoundingBox;
class Sphere;
class Matrix4;

enum ShapeType
{
    ShapeTypeBox = 0,
    ShapeTypeSphere,
    ShapeTypeCapsule,
    ShapeTypeConvexHull
};

class CollisionShape
{
private:
    ShapeType shapeType_;
    std::shared_ptr<ConvexHull> hullShape_;
    std::shared_ptr<BoundingBox> boxShape_;
    std::shared_ptr<Sphere> sphereShape_;
public:
    CollisionShape() :
        shapeType_(ShapeTypeBox)
    {}
    ~CollisionShape() = default;

    void SetConvexHull(const std::vector<Vector3>& vertices);
    void SetBox(const Vector3& min, const Vector3& max);
    void SetBox(float min, float max);
    void SetSphere(const Vector3& center, float radius);

    ShapeType GetShapeType() const { return shapeType_; }
    /// AABB
    BoundingBox GetWorldBoundingBox(const Matrix4& transform) const;
};

}
