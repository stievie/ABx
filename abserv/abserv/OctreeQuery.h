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
public:
    OctreeQuery(std::vector<Game::GameObject*>& result) :
        result_(result)
    { }
    virtual ~OctreeQuery() = default;
    OctreeQuery(const OctreeQuery& rhs) = delete;
    OctreeQuery& operator =(const OctreeQuery& rhs) = delete;

    /// Intersection test for an octant.
    virtual Intersection TestOctant(const BoundingBox& box, bool inside) = 0;
    /// Intersection test for objects.
    virtual void TestDrawables(Game::GameObject** start, Game::GameObject** end, bool inside) = 0;

    std::vector<Game::GameObject*>& result_;
};

class PointOctreeQuery : public OctreeQuery
{
public:
    PointOctreeQuery(std::vector<Game::GameObject*>& result, const Vector3 point) :
        OctreeQuery(result),
        point_(point)
    { }

    /// Intersection test for an octant.
    Intersection TestOctant(const BoundingBox& box, bool inside) override;
    /// Intersection test for objects.
    void TestDrawables(Game::GameObject** start, Game::GameObject** end, bool inside) override;

    Vector3 point_;
};

class SphereOctreeQuery : public OctreeQuery
{
public:
    /// Construct with sphere and query parameters.
    SphereOctreeQuery(std::vector<Game::GameObject*>& result, const Sphere& sphere) :
        OctreeQuery(result),
        sphere_(sphere)
    {}

    /// Intersection test for an octant.
    Intersection TestOctant(const BoundingBox& box, bool inside) override;
    /// Intersection test for objects.
    void TestDrawables(Game::GameObject** start, Game::GameObject** end, bool inside) override;

    /// Sphere.
    Sphere sphere_;
};

class BoxOctreeQuery : public OctreeQuery
{
public:
    /// Construct with bounding box and query parameters.
    BoxOctreeQuery(std::vector<Game::GameObject*>& result, const BoundingBox& box) :
        OctreeQuery(result),
        box_(box)
    {}

    /// Intersection test for an octant.
    Intersection TestOctant(const BoundingBox& box, bool inside) override;
    /// Intersection test for objects.
    void TestDrawables(Game::GameObject** start, Game::GameObject** end, bool inside) override;

    /// Bounding box.
    BoundingBox box_;
};

struct RayQueryResult
{
    /// Construct with defaults.
    RayQueryResult() :
        object_(nullptr)
    {}

    /// Test for inequality, added to prevent GCC from complaining.
    bool operator !=(const RayQueryResult& rhs) const
    {
        return position_ != rhs.position_ ||
            normal_ != rhs.normal_ ||
            distance_ != rhs.distance_ ||
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
        float maxDistance = INFINITY) :
        result_(result),
        ray_(ray),
        maxDistance_(maxDistance)
    {}

    RayOctreeQuery(const RayOctreeQuery& rhs) = delete;
    RayOctreeQuery& operator =(const RayOctreeQuery& rhs) = delete;

    /// Result vector reference.
    std::vector<RayQueryResult>& result_;
    /// Ray.
    Ray ray_;
    /// Drawable flags to include.
    unsigned char drawableFlags_;
    /// Drawable layers to include.
    unsigned viewMask_;
    /// Maximum ray distance.
    float maxDistance_;
};

}
