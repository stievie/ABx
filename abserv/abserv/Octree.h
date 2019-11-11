#pragma once

#include <limits>
#include "BoundingBox.h"
#include "Vector3.h"
#include "OctreeQuery.h"

namespace Game {
class GameObject;
}

namespace Math {

constexpr int NUM_OCTANTS = 8;
constexpr unsigned ROOT_INDEX = std::numeric_limits<unsigned>::max();

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
    void GetObjectsOnlyInternal(RayOctreeQuery& query, std::vector<Game::GameObject*>& objects) const;
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
    std::vector<Game::GameObject*> objects_;
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
    std::vector<Game::GameObject*> objectUpdate_;
private:
    /// Ray query temporary list of drawables.
    mutable std::vector<Game::GameObject*> rayQueryObjects_;
};

}
