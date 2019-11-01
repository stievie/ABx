#pragma once

#include "BoundingBox.h"
#include "Vector3.h"
#include "Sphere.h"
#include "Ray.h"

namespace Game {
class GameObject;
}

namespace Math {

class OctreeQuery
{
protected:
    const Game::GameObject* ignore_;
public:
    explicit OctreeQuery(std::vector<Game::GameObject*>& result, const Game::GameObject* ignore) :
        ignore_(ignore),
        result_(result)
    { }
    virtual ~OctreeQuery();
    OctreeQuery(const OctreeQuery& rhs) = delete;
    OctreeQuery& operator =(const OctreeQuery& rhs) = delete;

    /// Intersection test for an octant.
    virtual Intersection TestOctant(const BoundingBox& box, bool inside) = 0;
    /// Intersection test for objects.
    virtual void TestObjects(Game::GameObject** start, Game::GameObject** end, bool inside) = 0;

    std::vector<Game::GameObject*>& result_;
};

class PointOctreeQuery : public OctreeQuery
{
public:
    PointOctreeQuery(std::vector<Game::GameObject*>& result,
        const Vector3 point, const Game::GameObject* ignore = nullptr) :
        OctreeQuery(result, ignore),
        point_(point)
    { }

    /// Intersection test for an octant.
    Intersection TestOctant(const BoundingBox& box, bool inside) override;
    /// Intersection test for objects.
    void TestObjects(Game::GameObject** start, Game::GameObject** end, bool inside) override;

    Vector3 point_;
};

class SphereOctreeQuery : public OctreeQuery
{
public:
    /// Construct with sphere and query parameters.
    SphereOctreeQuery(std::vector<Game::GameObject*>& result,
        const Sphere& sphere, const Game::GameObject* ignore = nullptr) :
        OctreeQuery(result, ignore),
        sphere_(sphere)
    {}

    /// Intersection test for an octant.
    Intersection TestOctant(const BoundingBox& box, bool inside) override;
    /// Intersection test for objects.
    void TestObjects(Game::GameObject** start, Game::GameObject** end, bool inside) override;

    /// Sphere.
    Sphere sphere_;
};

class BoxOctreeQuery : public OctreeQuery
{
public:
    /// Construct with bounding box and query parameters.
    BoxOctreeQuery(std::vector<Game::GameObject*>& result,
        const BoundingBox& box, const Game::GameObject* ignore = nullptr) :
        OctreeQuery(result, ignore),
        box_(box)
    {}

    /// Intersection test for an octant.
    Intersection TestOctant(const BoundingBox& box, bool inside) override;
    /// Intersection test for objects.
    void TestObjects(Game::GameObject** start, Game::GameObject** end, bool inside) override;

    /// Bounding box.
    BoundingBox box_;
};

struct RayQueryResult
{
    /// Construct with defaults.
    RayQueryResult() :
        distance_(0.0f),
        object_(nullptr)
    {}
    RayQueryResult(const RayQueryResult& other) :
        position_(other.position_),
        normal_(other.normal_),
        distance_(other.distance_),
        object_(other.object_)
    { }
    RayQueryResult(RayQueryResult&& other) noexcept :
        position_(std::move(other.position_)),
        normal_(std::move(other.normal_)),
        distance_(other.distance_),
        object_(other.object_)
    {
        other.object_ = nullptr;
    }

    RayQueryResult& operator= (const RayQueryResult& other)
    {
        if (this != &other)
        {
            position_ = other.position_;
            normal_ = other.normal_;
            distance_ = other.distance_;
            object_ = other.object_;
        }
        return *this;
    }
    RayQueryResult& operator= (RayQueryResult&& other) noexcept
    {
        if (this != &other)
        {
            position_ = std::move(other.position_);
            normal_ = std::move(other.normal_);
            distance_ = other.distance_;
            object_ = other.object_;
            other.object_ = nullptr;
        }
        return *this;
    }

    /// Test for inequality, added to prevent GCC from complaining.
    bool operator !=(const RayQueryResult& rhs) const
    {
        return position_ != rhs.position_ ||
            normal_ != rhs.normal_ ||
            !Equals(distance_, rhs.distance_) ||
            object_ != rhs.object_;
    }

    /// Hit position in world space.
    Vector3 position_;
    /// Hit normal in world space. Negation of ray direction if per-triangle data not available.
    Vector3 normal_;
    /// Distance from ray origin.
    float distance_;
    /// Drawable.
    Game::GameObject* object_;
};

class RayOctreeQuery
{
public:
    /// Construct with ray and query parameters.
    RayOctreeQuery(std::vector<RayQueryResult>& result, const Ray& ray,
        float maxDistance = Math::M_INFINITE,
        const Game::GameObject* ignore = nullptr) :
        result_(result),
        ray_(ray),
        maxDistance_(maxDistance),
        ignore_(ignore)
    { }

    RayOctreeQuery(const RayOctreeQuery& rhs) = delete;
    RayOctreeQuery& operator =(const RayOctreeQuery& rhs) = delete;

    /// Result vector reference.
    std::vector<RayQueryResult>& result_;
    /// Ray.
    Ray ray_;
    /// Maximum ray distance.
    float maxDistance_;
    const Game::GameObject* ignore_;
};

}
