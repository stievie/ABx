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

#include "OctreeQuery.h"
#include <absmath/BoundingBox.h>
#include <absmath/Vector3.h>
#include <limits>
#include <eastl.hpp>

namespace Game {
class GameObject;
}

namespace Math {

inline constexpr int NUM_OCTANTS = 8;
inline constexpr unsigned ROOT_INDEX = std::numeric_limits<unsigned>::max();

class Octree;

class Octant
{
public:
    Octant(const BoundingBox& box, unsigned level, Octant* parent, Octree* root, unsigned index = ROOT_INDEX);
    virtual ~Octant();
    /// Return or create a child octant.
    Octant* GetOrCreateChild(unsigned index);
    /// Insert a drawable object by checking for fit recursively.
    void InsertObject(Game::GameObject* object);
    /// Reset root pointer recursively. Called when the whole octree is being destroyed.
    void ResetRoot();
    /// Delete child octant.
    void DeleteChild(unsigned index);
    /// Check if a drawable object fits.
    bool CheckObjectFit(const BoundingBox& box) const;
    /// Add a drawable object to this octant.
    void AddObject(Game::GameObject* object);
    /// Remove a drawable object from this octant.
    void RemoveObject(Game::GameObject* object, bool resetOctant = true);
    Octree* GetRoot() const { return root_; }
    const BoundingBox& GetCullingBox() const { return cullingBox_; }
protected:
    void Initialize(const BoundingBox& box);
    /// Return drawable objects by a query, called internally.
    void GetObjectsInternal(OctreeQuery& query, bool inside) const;
    /// Return drawable objects by a ray query, called internally.
    void GetObjectsInternal(RayOctreeQuery& query) const;
    /// Return drawable objects only for a threaded ray query, called internally.
    void GetObjectsOnlyInternal(RayOctreeQuery& query, ea::vector<Game::GameObject*>& objects) const;
    BoundingBox worldBoundingBox_;
    /// Bounding box used for drawable object fitting.
    BoundingBox cullingBox_;
    Vector3 center_;
    Vector3 halfSize_;
    /// Subdivision level
    uint32_t level_;
    Octant* children_[NUM_OCTANTS];
    Octant* parent_;
    Octree* root_;
    /// Octant index relative to its siblings or ROOT_INDEX for root octant
    unsigned index_;
    ea::vector<Game::GameObject*> objects_;
    /// Number of objects in this octant and child octants.
    unsigned numObjects_;
    /// Increase drawable object count recursively.
    void IncObjectCount()
    {
        ++numObjects_;
        if (parent_)
            parent_->IncObjectCount();
    }
    /// Decrease drawable object count recursively and remove octant if it becomes empty.
    void DecObjectCount()
    {
        Octant* parent = parent_;

        --numObjects_;
        if (!numObjects_)
        {
            if (parent)
                parent->DeleteChild(index_);
        }

        if (parent)
            parent->DecObjectCount();
    }
};

class Octree : public Octant
{
public:
    Octree();
    virtual ~Octree() override;
    void SetSize(const BoundingBox& box, unsigned numLevels);
    void Update();
    void AddObjectUpdate(Game::GameObject* object);
    void RemoveObjectUpdate(Game::GameObject* object);
    /// Return drawable objects by a query.
    void GetObjects(OctreeQuery& query) const;
    /// Return drawable objects by a ray query.
    void Raycast(RayOctreeQuery& query) const;
    /// Return the closest drawable object by a ray query.
    void RaycastSingle(RayOctreeQuery& query) const;

    /// Subdivision level.
    unsigned numLevels_;
    ea::vector<Game::GameObject*> objectUpdate_;
private:
    /// Ray query temporary list of drawables.
    mutable ea::vector<Game::GameObject*> rayQueryObjects_;
};

}
