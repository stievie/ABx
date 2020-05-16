/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <absmath/BoundingBox.h>
#include <absmath/Vector3.h>
#include <absmath/Sphere.h>
#include <absmath/Ray.h>
#include <eastl.hpp>

namespace Game {
class GameObject;
}

namespace Math {

class OctreeQuery
{
protected:
    const Game::GameObject* ignore_;
public:
    explicit OctreeQuery(ea::vector<Game::GameObject*>& result, const Game::GameObject* ignore) :
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

    ea::vector<Game::GameObject*>& result_;
};

class PointOctreeQuery : public OctreeQuery
{
public:
    PointOctreeQuery(ea::vector<Game::GameObject*>& result,
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
    SphereOctreeQuery(ea::vector<Game::GameObject*>& result,
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
    BoxOctreeQuery(ea::vector<Game::GameObject*>& result,
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
    RayQueryResult() noexcept = default;
    RayQueryResult(const RayQueryResult& other) noexcept :
        position_(other.position_),
        normal_(other.normal_),
        distance_(other.distance_),
        object_(other.object_)
    { }
    RayQueryResult(RayQueryResult&& other) noexcept :
        position_(std::move(other.position_)),
        normal_(std::move(other.normal_)),
        distance_(std::move(other.distance_)),
        object_(std::move(other.object_))
    { }

    RayQueryResult& operator= (const RayQueryResult& other) noexcept
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
    float distance_{ 0.0f };
    /// Drawable.
    Game::GameObject* object_{ nullptr };
};

class RayOctreeQuery
{
public:
    /// Construct with ray and query parameters.
    RayOctreeQuery(ea::vector<RayQueryResult>& result, const Ray& ray,
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
    ea::vector<RayQueryResult>& result_;
    /// Ray.
    Ray ray_;
    /// Maximum ray distance.
    float maxDistance_;
    const Game::GameObject* ignore_;
};

}
