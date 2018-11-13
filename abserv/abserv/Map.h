#pragma once

#include "NavigationMesh.h"
#include "Octree.h"
#include "Terrain.h"
#include "Vector3.h"
#include <pugixml.hpp>

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
};

/// Holds all the map data, static objects, NavMesh.
class Map
{
private:
    std::mutex lock_;
    std::weak_ptr<Game> game_;
    ai::Zone zone_;
    ai::ReadWriteLock aiLock_;
    // TerrainPatches are also owned by the game
    std::vector<std::shared_ptr<TerrainPatch>> patches_;
    typedef std::unordered_map<uint32_t, ai::AIPtr> Entities;
    Entities entities_;
public:
    explicit Map(std::shared_ptr<Game> game);
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
    inline const ai::Zone& GetZone() const
    {
        return zone_;
    }

    inline ai::Zone& GetZone()
    {
        return zone_;
    }
    inline int GetSize() const {
        // ???
        return 600;
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

    void LoadSceneNode(const pugi::xml_node& node);
    void AddGameObject(std::shared_ptr<GameObject> object);
    void Update(uint32_t delta);
    SpawnPoint GetFreeSpawnPoint();

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
