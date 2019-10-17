#include "stdafx.h"
#include "Map.h"
#include "DataProvider.h"
#include "Game.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Profiler.h"
#include "IOMap.h"
#include "Model.h"
#include "StringUtils.h"

namespace Game {

Map::Map(std::shared_ptr<Game> game, const std::string name) :
    game_(game),
    zone_(name),
    aiLock_("gamemap"),
    navMesh_(nullptr),
    octree_(std::make_unique<Math::Octree>())
{
    auto* aiServer = GetSubsystem<ai::Server>();
    if (aiServer)
        aiServer->addZone(&zone_);
}

Map::~Map()
{
    auto* aiServer = GetSubsystem<ai::Server>();
    if (aiServer)
        aiServer->removeZone(&zone_);
}

void Map::CreatePatches()
{
    patches_.clear();
    terrain_->numPatches_.x_ = (terrain_->GetHeightMap()->GetWidth() - 1) / terrain_->patchSize_;
    terrain_->numPatches_.y_ = (terrain_->GetHeightMap()->GetHeight() - 1) / terrain_->patchSize_;
    patches_.reserve(static_cast<size_t>(terrain_->numPatches_.x_) * static_cast<size_t>(terrain_->numPatches_.y_));
    for (int y = 0; y < terrain_->numPatches_.y_; ++y)
    {
        for (int x = 0; x < terrain_->numPatches_.x_; ++x)
        {
            patches_.push_back(
                std::make_shared<TerrainPatch>(terrain_, Math::Point<int>(x, y),
                    Math::Point<int>(terrain_->patchSize_, terrain_->patchSize_))
            );
        }
    }
}

TerrainPatch* Map::GetPatch(unsigned index) const
{
    return index < patches_.size() ? patches_[index].get() : nullptr;
}

TerrainPatch* Map::GetPatch(int x, int z) const
{
    if (x < 0 || x >= terrain_->numPatches_.x_ || z < 0 || z >= terrain_->numPatches_.y_)
        return nullptr;
    else
        return GetPatch((unsigned)(z * terrain_->numPatches_.x_ + x));
}

void Map::AddGameObject(std::shared_ptr<GameObject> object)
{
    if (auto game = game_.lock())
        game->AddObjectInternal(object);
}

void Map::UpdateAi(uint32_t delta)
{
    zone_.update(delta);
    auto* aiServer = GetSubsystem<ai::Server>();
    if (aiServer)
        aiServer->update(delta);
}

void Map::UpdateOctree(uint32_t)
{
    octree_->Update();
}

SpawnPoint Map::GetFreeSpawnPoint()
{
    return GetFreeSpawnPoint(spawnPoints_);
}

SpawnPoint Map::GetFreeSpawnPoint(const std::string& group)
{
    auto sps = GetSpawnPoints(group);
    if (sps.size() == 0)
        // Nothing found, return any spawn point
        return GetFreeSpawnPoint();

    return GetFreeSpawnPoint(sps);
}

SpawnPoint Map::GetFreeSpawnPoint(const std::vector<SpawnPoint>& points)
{
    if (points.size() == 0)
        return EmtpySpawnPoint;

    auto cleanObjects = [](std::vector<GameObject*>& objects)
    {
        if (objects.size() == 0)
            return;
        // Remove all objects that are not interesting
        objects.erase(std::remove_if(objects.begin(), objects.end(), [](GameObject* current)
        {
            return (current->GetCollisionMask() == 0) ||
                (current->GetType() == AB::GameProtocol::ObjectTypeTerrainPatch);
        }), objects.end());
    };

    size_t minObjects = std::numeric_limits<size_t>::max();
    SpawnPoint minPos;
    for (const auto& p : points)
    {
        std::vector<GameObject*> result;
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
        std::vector<GameObject*> result;
        Math::SphereOctreeQuery query(result, Math::Sphere(minPos.position, 1.0f));
        octree_->GetObjects(query);
        cleanObjects(result);
        while (result.size() != 0)
        {
#ifdef DEBUG_GAME
//            LOG_DEBUG << "In place " << result.size() << " Object: " << result.front()->GetName() <<
//                " Type: " << static_cast<int>(result.front()->GetType()) << std::endl;
#endif
            query.sphere_.center_.x_ += 0.2f;
            query.sphere_.center_.z_ += 0.2f;
            query.sphere_.center_.y_ = terrain_->GetHeight(query.sphere_.center_);
            octree_->GetObjects(query);
            cleanObjects(result);
        }
        return{ query.sphere_.center_, minPos.rotation, minPos.group };
    }
}

const SpawnPoint& Map::GetSpawnPoint(const std::string& group) const
{
    if (spawnPoints_.size() == 0)
        return EmtpySpawnPoint;
    for (const auto& sp : spawnPoints_)
    {
        if (sp.group.compare(group) == 0)
            return sp;
    }
    return EmtpySpawnPoint;
}

std::vector<SpawnPoint> Map::GetSpawnPoints(const std::string& group)
{
    std::vector<SpawnPoint> result;
    for (const auto& sp : spawnPoints_)
    {
        if (sp.group.compare(group) == 0)
            result.push_back(sp);
    }
    return result;
}

bool Map::FindPath(std::vector<Math::Vector3>& dest,
    const Math::Vector3& start, const Math::Vector3& end,
    const Math::Vector3& extends /* = Math::Vector3::One */,
    const dtQueryFilter* filter /* = nullptr */)
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
    const dtQueryFilter* filter, dtPolyRef* nearestRef)
{
    if (!navMesh_)
        return point;

    Math::Vector3 res = navMesh_->FindNearestPoint(point, extents, filter, nearestRef);
    res.y_ = GetTerrainHeight(res);
    return res;
}

}
