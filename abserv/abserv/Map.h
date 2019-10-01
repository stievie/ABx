#pragma once

#include <ai/AI.h>
#include <ai/zone/Zone.h>
#include <ai/common/Random.h>
#include "NavigationMesh.h"
#include "Octree.h"
#include "Terrain.h"
#include "Vector3.h"
#include <pugixml.hpp>
#include "TerrainPatch.h"

namespace Game {

class Game;

/// Database map data
struct MapData
{
    /// DB ID
    uint32_t id;
    /// The name of the map
    std::string name;
    std::string directory;
};

struct SpawnPoint
{
    Math::Vector3 position;
    Math::Quaternion rotation;
    std::string group;
    // equality comparison. doesn't modify object. therefore const.
    bool operator==(const SpawnPoint& rhs) const
    {
        return (position == rhs.position && rotation == rhs.rotation);
    }
    inline bool Empty() const;
};

static const SpawnPoint EmtpySpawnPoint{ Math::Vector3::Zero, Math::Quaternion::Identity, "" };
inline bool SpawnPoint::Empty() const
{
    return *this == EmtpySpawnPoint;
}


/// Holds all the map data, static objects, NavMesh.
class Map
{
private:
    std::weak_ptr<Game> game_;
    ai::Zone zone_;
    ai::ReadWriteLock aiLock_;
    // TerrainPatches are also owned by the game
    std::vector<std::shared_ptr<TerrainPatch>> patches_;
    typedef std::unordered_map<uint32_t, ai::AIPtr> Entities;
    Entities entities_;
public:
    Map(std::shared_ptr<Game> game, const std::string name);
    Map() = delete;
    ~Map();

    void CreatePatches();
    /// Return patch by index.
    TerrainPatch* GetPatch(unsigned index) const;
    /// Return patch by patch coordinates.
    TerrainPatch* GetPatch(int x, int z) const;
    size_t GetPatchesCount() const
    {
        return patches_.size();
    }
    float GetTerrainHeight(const Math::Vector3& world) const
    {
        if (terrain_)
            return terrain_->GetHeight(world);
        return 0.0f;
    }
    const ai::Zone& GetZone() const
    {
        return zone_;
    }
    ai::Zone& GetZone()
    {
        return zone_;
    }
    std::shared_ptr<Game> GetGame()
    {
        return game_.lock();
    }

    inline void SetEntityGroupId(const ai::AIPtr& entity, uint32_t oldGroupId, uint32_t groupId)
    {
        ai::GroupMgr& groupMgr = zone_.getGroupMgr();
        groupMgr.remove(oldGroupId, entity);
        entity->getCharacter()->setAttribute(ai::attributes::GROUP, std::to_string(groupId));
        groupMgr.add(groupId, entity);
    }
    inline void AddEntity(const ai::AIPtr& entity, uint32_t groupId)
    {
        if (!entity)
            return;

        {
            ai::ScopedReadLock lock(aiLock_);
            entities_.insert(std::make_pair(entity->getId(), entity));
        }
        ai_assert_always(zone_.addAI(entity), "Could not add entity to zone with id %i", entity->getId());
        ai::GroupMgr& groupMgr = zone_.getGroupMgr();
        entity->getCharacter()->setAttribute(ai::attributes::GROUP, std::to_string(groupId));
        groupMgr.add(groupId, entity);
        ai::AggroMgr& aggroMgr = entity->getAggroMgr();
        for (auto i : entities_)
        {
            const float rndAggro = ai::randomf(1000.0f);
            const float reductionVal = ai::randomf(2.0f);
            ai::Entry* e = aggroMgr.addAggro(i.second->getId(), 1000.0f + rndAggro);
            e->setReduceByValue(1.0f + reductionVal);
        }
    }
    inline bool RemoveEntity(uint32_t id)
    {
        ai::ScopedReadLock lock(aiLock_);
        auto iter = entities_.find(id);
        if (iter == entities_.end())
            return false;
        zone_.removeAI(iter->second);
        entities_.erase(iter);
        return true;
    }

    void AddGameObject(std::shared_ptr<GameObject> object);
    void UpdateAi(uint32_t delta);
    void UpdateOctree(uint32_t delta);
    SpawnPoint GetFreeSpawnPoint();
    SpawnPoint GetFreeSpawnPoint(const std::string& group);
    SpawnPoint GetFreeSpawnPoint(const std::vector<SpawnPoint>& points);
    const SpawnPoint& GetSpawnPoint(const std::string& group) const;
    std::vector<SpawnPoint> GetSpawnPoints(const std::string& group);

    /// Find a path between world space points. Return non-empty list of points if successful.
    /// Extents specifies how far off the navigation mesh the points can be.
    bool FindPath(std::vector<Math::Vector3>& dest, const Math::Vector3& start, const Math::Vector3& end,
        const Math::Vector3& extends = Math::Vector3::One, const dtQueryFilter* filter = nullptr);
    Math::Vector3 FindNearestPoint(const Math::Vector3& point, const Math::Vector3& extents,
        const dtQueryFilter* filter = nullptr, dtPolyRef* nearestRef = nullptr);

    MapData data_;
    std::vector<SpawnPoint> spawnPoints_;
    std::shared_ptr<Navigation::NavigationMesh> navMesh_;
    std::shared_ptr<Terrain> terrain_;
    std::unique_ptr<Math::Octree> octree_;
};

}
