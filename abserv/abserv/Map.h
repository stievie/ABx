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
    // TerrainPatches are also owned by the game
    std::vector<std::shared_ptr<TerrainPatch>> patches_;
public:
    Map(std::shared_ptr<Game> game);
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

    void LoadSceneNode(const pugi::xml_node& node);
    void AddGameObject(std::shared_ptr<GameObject> object);
    void Update(uint32_t delta);
    SpawnPoint GetFreeSpawnPoint();
    MapData data_;
    std::vector<SpawnPoint> spawnPoints_;
    std::shared_ptr<NavigationMesh> navMesh_;
    std::shared_ptr<Terrain> terrain_;
    std::unique_ptr<Math::Octree> octree_;
};

}
