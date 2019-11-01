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
#include <limits>

namespace Math {

enum class ShapeType
{
    BoundingBox,
    Sphere,
    ConvexHull,
    HeightMap,

    None = 99
};

struct CollisionManifold
{
    // Input
    Vector3 position;
    Vector3 velocity;
    Vector3 radius;

    // Output
    Vector3 normalizedVelocity;
    Vector3 nearestSphereIntersectionPoint;
    Vector3 intersectionPoint;
    Vector3 normal;
    float distance{ M_INFINITE };
    bool stuck{ false };
};

class AbstractCollisionShape
{
public:
    explicit AbstractCollisionShape(ShapeType type) :
        shapeType_(type)
    {}
    virtual ~AbstractCollisionShape();

    /// AABB
    virtual BoundingBox GetWorldBoundingBox(const Matrix4& transform) const = 0;
    virtual BoundingBox GetBoundingBox() const = 0;

    virtual bool Collides(const Matrix4& transformation, const BoundingBox& other, const Vector3& velocity, Vector3& move) const = 0;
    virtual bool Collides(const Matrix4& transformation, const Sphere& other, const Vector3& velocity, Vector3& move) const = 0;
    virtual bool Collides(const Matrix4& transformation, const HeightMap& other, const Vector3& velocity, Vector3& move) const = 0;
    virtual bool Collides(const Matrix4& transformation, const ConvexHull& other, const Vector3& velocity, Vector3& move) const = 0;

    virtual bool Collides(const BoundingBox& other, const Vector3& velocity, Vector3& move) const = 0;
    virtual bool Collides(const Sphere& other, const Vector3& velocity, Vector3& move) const = 0;
    virtual bool Collides(const HeightMap& other, const Vector3& velocity, Vector3& move) const = 0;
    virtual bool Collides(const ConvexHull& other, const Vector3& velocity, Vector3& move) const = 0;

    virtual Shape GetShape() const = 0;
    bool GetManifold(CollisionManifold&, const Matrix4& transformation) const;

    ShapeType shapeType_;
};

template <typename T>
class CollisionShape final : public AbstractCollisionShape
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
    /// Create a transformed version. Requires that T has a copy constructor.
    explicit CollisionShape(const CollisionShape<T>& other, const Matrix4& transformation) :
        AbstractCollisionShape(other.shapeType_),
        object_(std::make_shared<T>(other.Object().Transformed(transformation)))
    { }

    BoundingBox GetWorldBoundingBox(const Matrix4& transform) const override
    {
        assert(object_);
        return object_->GetBoundingBox().Transformed(transform);
    }

    BoundingBox GetBoundingBox() const override
    {
        assert(object_);
        return object_->GetBoundingBox();
    }

    // Object must be in local space
    /// @param transformation Our transformation matrix
    /// @param other The other object transformed to world coordinates
    /// @param velocity Our velocity
    /// @param move
    bool Collides(const Matrix4& transformation, const BoundingBox& other, const Vector3& velocity, Vector3& move) const override
    {
        assert(object_);
        return object_->Transformed(transformation).Collides(other, velocity, move);
    }
    bool Collides(const Matrix4& transformation, const Sphere& other, const Vector3& velocity, Vector3& move) const override
    {
        assert(object_);
        return object_->Transformed(transformation).Collides(other, velocity, move);
    }
    bool Collides(const Matrix4& transformation, const HeightMap& other, const Vector3& velocity, Vector3& move) const override
    {
        assert(object_);
        return object_->Transformed(transformation).Collides(other, velocity, move);
    }
    bool Collides(const Matrix4& transformation, const ConvexHull& other, const Vector3& velocity, Vector3& move) const override
    {
        assert(object_);
        return object_->Transformed(transformation).Collides(other, velocity, move);
    }

    // Object must be transformed to world coordinates
    bool Collides(const BoundingBox& other, const Vector3& velocity, Vector3& move) const override
    {
        assert(object_);
        return object_->Collides(other, velocity, move);
    }
    bool Collides(const Sphere& other, const Vector3& velocity, Vector3& move) const override
    {
        assert(object_);
        return object_->Collides(other, velocity, move);
    }
    bool Collides(const HeightMap& other, const Vector3& velocity, Vector3& move) const override
    {
        assert(object_);
        return object_->Collides(other, velocity, move);
    }
    bool Collides(const ConvexHull& other, const Vector3& velocity, Vector3& move) const override
    {
        assert(object_);
        return object_->Collides(other, velocity, move);
    }

    const T& Object() const
    {
        assert(object_);
        return *object_;
    }
    T& Object()
    {
        assert(object_);
        return *object_;
    }

    Shape GetShape() const override
    {
        assert(object_);
        return object_->GetShape();
    }
};

}
