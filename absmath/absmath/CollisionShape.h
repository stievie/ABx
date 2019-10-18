#pragma once

#include "Vector3.h"
#include "Matrix4.h"
#include "MathDefs.h"
#include "BoundingBox.h"
#include "Sphere.h"
#include "HeightMap.h"
#include "ConvexHull.h"
#include "Shape.h"
#include <memory>

namespace Math {

enum class ShapeType
{
    BoundingBox,
    Sphere,
    ConvexHull,
    HeightMap,

    None = 99
};

class AbstractCollisionShape
{
public:
    AbstractCollisionShape(ShapeType type) :
        shapeType_(type)
    {}
    virtual ~AbstractCollisionShape() = default;

    /// AABB
    virtual BoundingBox GetWorldBoundingBox(const Matrix4& transform) const = 0;
    virtual BoundingBox GetBoundingBox() const = 0;

    virtual bool Collides(const Matrix4& transformation, const BoundingBox& other, const Vector3& velocity, Vector3& move) const = 0;
    virtual bool Collides(const Matrix4& transformation, const Sphere& other, const Vector3& velocity, Vector3& move) const = 0;
    virtual bool Collides(const Matrix4& transformation, const HeightMap& other, const Vector3& velocity, Vector3& move) const = 0;
    virtual bool Collides(const Matrix4& transformation, const ConvexHull& other, const Vector3& velocity, Vector3& move) const = 0;

    virtual Shape GetShape() const = 0;

    ShapeType shapeType_;
};

template <typename T>
class CollisionShape : public AbstractCollisionShape
{
private:
    std::shared_ptr<T> object_;
public:
    /// Ctor. Create new shape
    template<typename... _CArgs>
    explicit CollisionShape(ShapeType type, _CArgs&&... _Args) :
        AbstractCollisionShape(type),
        object_(std::make_shared<T>(std::forward<_CArgs>(_Args)...))
    { }
    /// Ctor. Assign existing shape
    explicit CollisionShape(ShapeType type, std::shared_ptr<T> ptr) :
        AbstractCollisionShape(type),
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

    bool Collides(const Matrix4& transformation, const BoundingBox& other, const Vector3& velocity, Vector3& move) const override
    {
        return object_->Transformed(transformation).Collides(other, velocity, move);
    }
    bool Collides(const Matrix4& transformation, const Sphere& other, const Vector3& velocity, Vector3& move) const override
    {
        return object_->Transformed(transformation).Collides(other, velocity, move);
    }
    bool Collides(const Matrix4& transformation, const HeightMap& other, const Vector3& velocity, Vector3& move) const override
    {
        return object_->Transformed(transformation).Collides(other, velocity, move);
    }
    bool Collides(const Matrix4& transformation, const ConvexHull& other, const Vector3& velocity, Vector3& move) const override
    {
        return object_->Transformed(transformation).Collides(other, velocity, move);
    }

    T* Object() const
    {
        if (object_)
            return object_.get();
        return nullptr;
    }

    Shape GetShape() const override
    {
        assert(object_);
        return object_->GetShape();
    }
};

}
