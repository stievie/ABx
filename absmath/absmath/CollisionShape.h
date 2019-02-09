#pragma once

#include "Vector3.h"
#include "Matrix4.h"
#include "MathDefs.h"
#include "BoundingBox.h"
#include "Sphere.h"
#include "HeightMap.h"
#include "ConvexHull.h"
#include "Shape.h"

namespace Math {

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
    virtual BoundingBox GetBoundingBox() const = 0;

    virtual bool Collides(const Matrix4& transformation, const BoundingBox& other, Vector3& move) const = 0;
    virtual bool Collides(const Matrix4& transformation, const Sphere& other, Vector3& move) const = 0;
    virtual bool Collides(const Matrix4& transformation, const HeightMap& other, Vector3& move) const = 0;
    virtual bool Collides(const Matrix4& transformation, const ConvexHull& other, Vector3& move) const = 0;

    virtual Shape GetShape() const = 0;

    ShapeType shapeType_;
};

template <typename T>
class CollisionShapeImpl : public CollisionShape
{
private:
    std::shared_ptr<T> object_;
public:
    /// Ctor. Create new shape
    template<typename... _CArgs>
    explicit CollisionShapeImpl(ShapeType type, _CArgs&&... _Args) :
        CollisionShape(type),
        object_(std::make_shared<T>(std::forward<_CArgs>(_Args)...))
    { }
    /// Ctor. Assign existing shape
    explicit CollisionShapeImpl(ShapeType type, std::shared_ptr<T> ptr) :
        CollisionShape(type),
        object_(ptr)
    { }

    BoundingBox GetWorldBoundingBox(const Matrix4& transform) const override
    {
        return object_->GetBoundingBox().Transformed(transform);
    }

    BoundingBox GetBoundingBox() const override
    {
        return object_->GetBoundingBox();
    }

    bool Collides(const Matrix4& transformation, const BoundingBox& other, Vector3& move) const override
    {
        return object_->Transformed(transformation).Collides(other, move);
    }
    bool Collides(const Matrix4& transformation, const Sphere& other, Vector3& move) const override
    {
        return object_->Transformed(transformation).Collides(other, move);
    }
    bool Collides(const Matrix4& transformation, const HeightMap& other, Vector3& move) const override
    {
        return object_->Transformed(transformation).Collides(other, move);
    }
    bool Collides(const Matrix4& transformation, const ConvexHull& other, Vector3& move) const override
    {
        return object_->Transformed(transformation).Collides(other, move);
    }

    T* Object() const
    {
        if (object_)
            return object_.get();
        return nullptr;
    }

    Shape GetShape() const override
    {
        return object_->GetShape();
    }
};

}
