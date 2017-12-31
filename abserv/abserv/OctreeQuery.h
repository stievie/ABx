#pragma once

#include "BoundingBox.h"
#include "Vector3.h"
#include "Sphere.h"

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
    /// Intersection test for drawables.
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
    /// Intersection test for drawables.
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
    /// Intersection test for drawables.
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
    /// Intersection test for drawables.
    void TestDrawables(Game::GameObject** start, Game::GameObject** end, bool inside) override;

    /// Bounding box.
    BoundingBox box_;
};

}
