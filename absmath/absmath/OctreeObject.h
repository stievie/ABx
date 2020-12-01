/**
 * Copyright 2020 Stefan Ascher
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

#include "BoundingBox.h"
#include <eastl.hpp>
#include <sa/Compiler.h>

namespace Math {

class Octant;
class RayOctreeQuery;
struct RayQueryResult;
class Octant;

class SA_NOVTABLE OctreeObject
{
protected:
    float sortValue_{ 0.0f };
    Octant* octant_{ nullptr };
    /// Occluder flag. An object that can hide another object from view.
    bool occluder_{ false };
    /// Occludee flag. An object that can be hidden from view (because it is occluded by another object) but that cannot, itself, hide another object from view.
    bool occludee_{ true };
    uint32_t collisionLayer_{ 1 };
    uint32_t collisionMask_{ 0xFFFFFFFF };     // Collides with all by default
public:
    virtual ~OctreeObject();
    void SetSortValue(float value) { sortValue_ = value; }
    float GetSortValue() const { return sortValue_; }
    Octant* GetOctant() const;
    void SetOctant(Octant* octant);
    bool IsOccludee() const { return occludee_; }
    void SetOccludee(bool value) { occludee_ = value; }
    bool IsOccluder() const { return occluder_; }
    void SetOccluder(bool value) { occluder_ = value; }
    void SetCollisionLayer(uint32_t value) { collisionLayer_ = value; }
    uint32_t GetCollsionLayer() const { return collisionLayer_; }
    void SetCollisionMask(uint32_t mask) { collisionMask_ = mask; }
    uint32_t GetCollisionMask() const { return collisionMask_; }
    bool CollisionMaskMatches(uint32_t layer) const { return (layer & collisionMask_); }

    virtual BoundingBox GetWorldBoundingBox() const = 0;
    virtual void ProcessRayQuery(const RayOctreeQuery& query, ea::vector<RayQueryResult>& results) = 0;
};

inline bool CompareObjects(OctreeObject* lhs, OctreeObject* rhs)
{
    return lhs->GetSortValue() < rhs->GetSortValue();
}

}