#pragma once

#include "MathDefs.h"
#include "BoundingBox.h"

namespace Math {

class Vector3;
class ConvexHull;
class HeightMap;
class Sphere;
class Matrix4;

enum ShapeType
{
    ShapeTypeBoundingBox,
    ShapeTypeSphere,
    ShapeTypeConvexHull,
    ShapeTypeHeightMap
};

class CollisionShape
{
public:
    CollisionShape(ShapeType type) :
        shapeType_(type)
    {}
    ~CollisionShape() = default;

    /// AABB
    virtual BoundingBox GetWorldBoundingBox(const Matrix4& transform) const = 0;

    virtual bool Collides(const Matrix4& transformation, const BoundingBox& other, Vector3& move) const = 0;
    virtual bool Collides(const Matrix4& transformation, const Sphere& other, Vector3& move) const = 0;
    virtual bool Collides(const Matrix4& transformation, const HeightMap& other, Vector3& move) const = 0;
    virtual bool Collides(const Matrix4& transformation, const ConvexHull& other, Vector3& move) const = 0;

    ShapeType shapeType_;
};

template <typename T>
class CollisionShapeImpl : public CollisionShape
{
public:
    template<typename... _CArgs>
    CollisionShapeImpl(ShapeType type, _CArgs&&... _Args) :
        CollisionShape(type),
        shape_(std::forward<_CArgs>(_Args)...)
    { }

    BoundingBox GetWorldBoundingBox(const Matrix4& transform) const override
    {
        return shape_.GetBoundingBox().Transformed(transform);
    }

    bool Collides(const Matrix4& transformation, const BoundingBox& other, Vector3& move) const override
    {
        return shape_.Transformed(transformation).Collides(other, move);
    }
    bool Collides(const Matrix4& transformation, const Sphere& other, Vector3& move) const override
    {
        return shape_.Transformed(transformation).Collides(other, move);
    }
    bool Collides(const Matrix4& transformation, const HeightMap& other, Vector3& move) const override
    {
        assert(false);
        return false;
    }
    bool Collides(const Matrix4& transformation, const ConvexHull& other, Vector3& move) const override
    {
        assert(false);
        return false;
    }

    T shape_;
};

}
