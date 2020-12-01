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


#include "Map.h"
#include "DataProvider.h"
#include "Game.h"
#include "IOMap.h"
#include "Model.h"

namespace Game {

Map::Map(ea::shared_ptr<Game> game) :
    game_(game),
    octree_(ea::make_unique<Math::Octree>())
{
}

Map::~Map()
{
}

void Map::CreatePatches()
{
    patches_.clear();
    terrain_->numPatches_.x_ = (terrain_->GetHeightMap()->numVertices_.x_ - 1) / terrain_->patchSize_;
    terrain_->numPatches_.y_ = (terrain_->GetHeightMap()->numVertices_.y_ - 1) / terrain_->patchSize_;
    patches_.reserve(static_cast<size_t>(terrain_->numPatches_.x_) * static_cast<size_t>(terrain_->numPatches_.y_));
    for (int y = 0; y < terrain_->numPatches_.y_; ++y)
    {
        for (int x = 0; x < terrain_->numPatches_.x_; ++x)
        {
            patches_.push_back(
                ea::make_shared<TerrainPatch>(terrain_, Math::IntVector2(x, y),
                    Math::IntVector2(terrain_->patchSize_, terrain_->patchSize_))
            );
        }
    }
}

TerrainPatch* Map::GetPatch(size_t index) const
{
    return index < patches_.size() ? patches_[index].get() : nullptr;
}

TerrainPatch* Map::GetPatch(int x, int z) const
{
    if (x < 0 || x >= terrain_->numPatches_.x_ || z < 0 || z >= terrain_->numPatches_.y_)
        return nullptr;
    else
        return GetPatch((size_t)(z * terrain_->numPatches_.x_ + x));
}

void Map::AddGameObject(ea::shared_ptr<GameObject> object)
{
    if (auto game = game_.lock())
        game->AddObjectInternal(object);
}

void Map::UpdateOctree(uint32_t)
{
    octree_->Update();
}

SpawnPoint Map::GetFreeSpawnPoint() const
{
    if (!scene_)
        return EmtpySpawnPoint;
    return GetFreeSpawnPoint(scene_->spawnPoints_);
}

SpawnPoint Map::GetFreeSpawnPoint(const std::string& group) const
{
    auto sps = GetSpawnPoints(group);
    if (sps.size() == 0)
        // Nothing found, return any spawn point
        return GetFreeSpawnPoint();

    return GetFreeSpawnPoint(sps);
}

SpawnPoint Map::GetFreeSpawnPoint(const ea::vector<SpawnPoint>& points) const
{
    if (points.size() == 0)
        return EmtpySpawnPoint;

    auto cleanObjects = [](ea::vector<Math::OctreeObject*>& objects)
    {
        if (objects.size() == 0)
            return;
        // Remove all objects that are not interesting
        objects.erase(ea::remove_if(objects.begin(), objects.end(), [](Math::OctreeObject* current)
        {
            return (current->GetCollisionMask() == 0) ||
                Is<TerrainPatch>(static_cast<GameObject*>(current));
        }), objects.end());
    };

    size_t minObjects = std::numeric_limits<size_t>::max();
    SpawnPoint minPos;
    for (const auto& p : points)
    {
        ea::vector<Math::OctreeObject*> result;
        Math::SphereOctreeQuery query(result, Math::Sphere(p.position, 5.0f));
        octree_->GetObjects(query);
        cleanObjects(result);
        if (result.size() < minObjects)
        {
            minPos = p;
            minObjects = result.size();
            if (minObjects == 0)
                break;
        }
    }

    {
        ea::vector<Math::OctreeObject*> result;
        Math::SphereOctreeQuery query(result, Math::Sphere(minPos.position, 1.0f));
        octree_->GetObjects(query);
        cleanObjects(result);
        while (result.size() != 0)
        {
#ifdef DEBUG_GAME
//            LOG_DEBUG << "In place " << result.size() << " Object: " << *result.front() << std::endl;
#endif
            query.sphere_.center_.x_ += 0.2f;
            query.sphere_.center_.z_ += 0.2f;
            query.sphere_.center_.y_ = terrain_->GetHeight(query.sphere_.center_);
            octree_->GetObjects(query);
            cleanObjects(result);
        }
        return CorrectedSpanwPoint({ query.sphere_.center_, minPos.rotation, minPos.group });
    }
}

SpawnPoint Map::GetSpawnPoint(const std::string& group) const
{
    if (!scene_)
        return EmtpySpawnPoint;
    if (scene_->spawnPoints_.size() == 0)
        return EmtpySpawnPoint;
    for (const auto& sp : scene_->spawnPoints_)
    {
        if (sp.group.compare(group) == 0)
            return CorrectedSpanwPoint(sp);
    }
    return EmtpySpawnPoint;
}

ea::vector<SpawnPoint> Map::GetSpawnPoints(const std::string& group) const
{
    ea::vector<SpawnPoint> result;
    for (const auto& sp : scene_->spawnPoints_)
    {
        if (sp.group.compare(group) == 0)
            result.push_back(CorrectedSpanwPoint(sp));
    }
    return result;
}

float Map::GetTerrainHeight(const Math::Vector3& world) const
{
    if (terrain_)
        return terrain_->GetHeight(world);
    return world.y_;
}

void Map::UpdatePointHeight(Math::Vector3& world) const
{
    world.y_ = GetTerrainHeight(world);
}

SpawnPoint Map::CorrectedSpanwPoint(const SpawnPoint& world) const
{
    SpawnPoint result = world;
    result.position.y_ = GetTerrainHeight(result.position);
    return result;
}

bool Map::FindPath(ea::vector<Math::Vector3>& dest,
    const Math::Vector3& start, const Math::Vector3& end,
    const Math::Vector3& extends /* = Math::Vector3::One */,
    const dtQueryFilter* filter /* = nullptr */) const
{
    if (!navMesh_)
        return false;

    const bool res = navMesh_->FindPath(dest, start, end, extends, filter);
    if (res)
    {
        for (auto& v : dest)
            v.y_ = GetTerrainHeight(v);
    }
    return res;
}

Math::Vector3 Map::FindNearestPoint(const Math::Vector3& point,
    const Math::Vector3& extents,
    const dtQueryFilter* filter, dtPolyRef* nearestRef) const
{
    if (!navMesh_)
        return point;

    Math::Vector3 res = navMesh_->FindNearestPoint(point, extents, filter, nearestRef);
    res.y_ = GetTerrainHeight(res);
    return res;
}

bool Map::CanStepOn(const Math::Vector3& point, const Math::Vector3& extents,
    const dtQueryFilter* filter, dtPolyRef* nearestRef, Math::Vector3* nearestPoint) const
{
    if (!navMesh_)
        return false;

    return navMesh_->CanStepOn(point, extents, filter, nearestRef, nearestPoint);
}

}
