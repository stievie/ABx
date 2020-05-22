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


#include "Octree.h"
#include "GameObject.h"

namespace Math {

static constexpr float DEFAULT_OCTREE_SIZE = 1000.0f;
static constexpr int DEFAULT_OCTREE_LEVELS = 8;

inline bool CompareRayQueryResults(const RayQueryResult& lhs, const RayQueryResult& rhs)
{
    return lhs.distance_ < rhs.distance_;
}

Octree::Octree() :
    Octant(BoundingBox(-DEFAULT_OCTREE_SIZE, DEFAULT_OCTREE_SIZE), 0, nullptr, this),
    numLevels_(DEFAULT_OCTREE_LEVELS)
{
}

Octree::~Octree()
{
    ResetRoot();
}

void Octree::SetSize(const BoundingBox& box, unsigned numLevels)
{
    // If object exist, they are temporarily moved to the root
    for (unsigned i = 0; i < NUM_OCTANTS; ++i)
        DeleteChild(i);

    Initialize(box);
    numObjects_ = static_cast<unsigned>(objects_.size());
    numLevels_ = std::max(numLevels, 1U);
}

void Octree::AddObjectUpdate(Game::GameObject* object)
{
    auto it = std::find(objectUpdate_.begin(), objectUpdate_.end(), object);
    if (it == objectUpdate_.end())
        objectUpdate_.push_back(object);
}

void Octree::RemoveObjectUpdate(Game::GameObject* object)
{
    auto it = std::find(objectUpdate_.begin(), objectUpdate_.end(), object);
    if (it != objectUpdate_.end())
    {
        objectUpdate_.erase(it);
    }
}

void Octree::GetObjects(OctreeQuery& query) const
{
    query.result_.clear();
    GetObjectsInternal(query, false);
}

void Octree::Raycast(RayOctreeQuery& query) const
{
    query.result_.clear();
    GetObjectsInternal(query);
    ea::sort(query.result_.begin(), query.result_.end(), CompareRayQueryResults);
}

void Octree::RaycastSingle(RayOctreeQuery& query) const
{
    query.result_.clear();
    rayQueryObjects_.clear();
    GetObjectsOnlyInternal(query, rayQueryObjects_);

    // Sort by increasing hit distance to AABB
    for (auto object : rayQueryObjects_)
    {
        object->SetSortValue(query.ray_.HitDistance(object->GetWorldBoundingBox()));
    }
    ea::sort(rayQueryObjects_.begin(), rayQueryObjects_.end(), Game::CompareObjects);

    // Then do the actual test according to the query, and early-out as possible
    float closestHit = Math::M_INFINITE;
    for (auto object : rayQueryObjects_)
    {
        if (object->GetSortValue() < std::min(closestHit, query.maxDistance_))
        {
            if (object == query.ignore_)
                continue;

            size_t oldSize = query.result_.size();
            object->ProcessRayQuery(query, query.result_);
            if (query.result_.size() > oldSize)
                closestHit = std::min(closestHit, query.result_.back().distance_);
        }
        else
            break;
    }

    if (query.result_.size() > 1)
    {
        std::sort(query.result_.begin(), query.result_.end(), CompareRayQueryResults);
        query.result_.resize(1);
    }
}

void Octree::Update()
{
    if (objectUpdate_.empty())
        return;

    for (Game::GameObject* o : objectUpdate_)
    {
        Octant* octant = o->GetOctant();
        const BoundingBox& box = o->GetWorldBoundingBox();
        // Skip if no octant or does not belong to this octree anymore
        if (!octant || octant->GetRoot() != this)
            continue;
        // Skip if still fits the current octant
        if (o->occludee_ && octant->GetCullingBox().IsInside(box) == Intersection::Inside && octant->CheckObjectFit(box))
            continue;

        InsertObject(o);
    }
    objectUpdate_.clear();
}

Octant::Octant(const BoundingBox& box, unsigned level, Octant* parent, Octree* root, unsigned index) :
    level_(level),
    parent_(parent),
    root_(root),
    index_(index)
{
    Initialize(box);
    for (unsigned i = 0; i < NUM_OCTANTS; ++i)
        children_[i] = nullptr;
}

Octant::~Octant()
{
    if (root_)
    {
        // Remove the objects (if any) from this octant to the root octant
        for (auto& o : objects_)
        {
            o->SetOctant(root_);
            root_->objects_.push_back(o);
        }
        objects_.clear();
        numObjects_ = 0;
    }

    for (unsigned i = 0; i < NUM_OCTANTS; ++i)
        DeleteChild(i);
}

Octant* Octant::GetOrCreateChild(unsigned index)
{
    if (children_[index])
        return children_[index];

    Vector3 newMin = worldBoundingBox_.min_;
    Vector3 newMax = worldBoundingBox_.max_;
    Vector3 oldCenter = worldBoundingBox_.Center();

    if (index & 1)
        newMin.x_ = oldCenter.x_;
    else
        newMax.x_ = oldCenter.x_;

    if (index & 2)
        newMin.y_ = oldCenter.y_;
    else
        newMax.y_ = oldCenter.y_;

    if (index & 4)
        newMin.z_ = oldCenter.z_;
    else
        newMax.z_ = oldCenter.z_;

    children_[index] = new Octant(BoundingBox(newMin, newMax), level_ + 1, this, root_, index);
    return children_[index];
}

void Octant::InsertObject(Game::GameObject* object)
{
    const BoundingBox& box = object->GetWorldBoundingBox();

    // If root octant, insert all non-occludees here, so that octant occlusion does not hide the object.
    // Also if object is outside the root octant bounds, insert to root
    bool insertHere;
    if (this == root_)
        insertHere = !object->occludee_ || cullingBox_.IsInside(box) != Intersection::Inside || CheckObjectFit(box);
    else
        insertHere = CheckObjectFit(box);

    if (insertHere)
    {
        Octant* oldOctant = object->octant_;
        if (oldOctant != this)
        {
            // Add first, then remove, because object count going to zero deletes the octree branch in question
            AddObject(object);
            if (oldOctant)
                oldOctant->RemoveObject(object, false);
        }
    }
    else
    {
        Vector3 boxCenter = box.Center();
        unsigned x = boxCenter.x_ < center_.x_ ? 0 : 1;
        unsigned y = boxCenter.y_ < center_.y_ ? 0 : 2;
        unsigned z = boxCenter.z_ < center_.z_ ? 0 : 4;

        GetOrCreateChild(x + y + z)->InsertObject(object);
    }
}

void Octant::ResetRoot()
{
    root_ = nullptr;

    for (auto& o : objects_)
    {
        o->SetOctant(nullptr);
    }

    for (unsigned i = 0; i < NUM_OCTANTS; ++i)
    {
        if (children_[i])
            children_[i]->ResetRoot();
    }
}

void Octant::DeleteChild(unsigned index)
{
    assert(index < NUM_OCTANTS);
    delete children_[index];
    children_[index] = nullptr;
}

bool Octant::CheckObjectFit(const BoundingBox& box) const
{
    Vector3 boxSize = box.Size();

    // If max split level, size always OK, otherwise check that box is at least half size of octant
    if (level_ >= root_->numLevels_ || boxSize.x_ >= halfSize_.x_ || boxSize.y_ >= halfSize_.y_ ||
        boxSize.z_ >= halfSize_.z_)
        return true;
    // Also check if the box can not fit a child octant's culling box, in that case size OK (must insert here)
    else
    {
        if (box.min_.x_ <= worldBoundingBox_.min_.x_ - 0.5f * halfSize_.x_ ||
            box.max_.x_ >= worldBoundingBox_.max_.x_ + 0.5f * halfSize_.x_ ||
            box.min_.y_ <= worldBoundingBox_.min_.y_ - 0.5f * halfSize_.y_ ||
            box.max_.y_ >= worldBoundingBox_.max_.y_ + 0.5f * halfSize_.y_ ||
            box.min_.z_ <= worldBoundingBox_.min_.z_ - 0.5f * halfSize_.z_ ||
            box.max_.z_ >= worldBoundingBox_.max_.z_ + 0.5f * halfSize_.z_)
            return true;
    }

    // Bounding box too small, should create a child octant
    return false;
}

void Octant::AddObject(Game::GameObject* object)
{
    object->SetOctant(this);
    objects_.push_back(object);
    IncObjectCount();
}

void Octant::RemoveObject(Game::GameObject* object, bool resetOctant /* = true */)
{
    root_->RemoveObjectUpdate(object);
    auto it = std::find(objects_.begin(), objects_.end(), object);
    if (it != objects_.end())
    {
        objects_.erase(it);
        if (resetOctant)
            object->SetOctant(nullptr);
        DecObjectCount();
    }
}

void Octant::Initialize(const BoundingBox& box)
{
    worldBoundingBox_ = box;
    center_ = box.Center();
    halfSize_ = 0.5f * box.Size();
    cullingBox_ = BoundingBox(worldBoundingBox_.min_ - halfSize_, worldBoundingBox_.max_ + halfSize_);
}

void Octant::GetObjectsInternal(OctreeQuery& query, bool inside) const
{
    if (this != root_)
    {
        Intersection res = query.TestOctant(cullingBox_, inside);
        if (res == Intersection::Inside)
            inside = true;
        else if (res == Intersection::Outside)
        {
            // Fully outside, so cull this octant, its children & objects
            return;
        }
    }

    if (objects_.size())
    {
        Game::GameObject** start = const_cast<Game::GameObject**>(&objects_[0]);
        Game::GameObject** end = start + objects_.size();
        query.TestObjects(start, end, inside);
    }

    for (unsigned i = 0; i < NUM_OCTANTS; ++i)
    {
        if (children_[i])
            children_[i]->GetObjectsInternal(query, inside);
    }
}

void Octant::GetObjectsInternal(RayOctreeQuery& query) const
{
    float octantDist = query.ray_.HitDistance(cullingBox_);
    if (octantDist >= query.maxDistance_)
        return;

    if (objects_.size())
    {
        Game::GameObject** start = const_cast<Game::GameObject**>(&objects_[0]);
        Game::GameObject** end = start + objects_.size();

        while (start != end)
        {
            Game::GameObject* object = *start++;
            if (object == query.ignore_)
                continue;
            object->ProcessRayQuery(query, query.result_);
        }
    }

    for (unsigned i = 0; i < NUM_OCTANTS; ++i)
    {
        if (children_[i])
            children_[i]->GetObjectsInternal(query);
    }
}

void Octant::GetObjectsOnlyInternal(RayOctreeQuery& query, ea::vector<Game::GameObject*>& objects) const
{
    float octantDist = query.ray_.HitDistance(cullingBox_);
    if (octantDist >= query.maxDistance_)
        return;

    if (objects_.size())
    {
        Game::GameObject** start = const_cast<Game::GameObject**>(&objects_[0]);
        Game::GameObject** end = start + objects_.size();

        while (start != end)
        {
            Game::GameObject* object = *start++;
            objects.push_back(object);
        }
    }

    for (unsigned i = 0; i < NUM_OCTANTS; ++i)
    {
        if (children_[i])
            children_[i]->GetObjectsOnlyInternal(query, objects);
    }
}

}
