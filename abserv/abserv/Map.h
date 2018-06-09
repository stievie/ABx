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
public:
    Map(std::shared_ptr<Game> game);
    virtual ~Map();

    void LoadSceneNode(const pugi::xml_node& node);
    void Update(uint32_t delta);
    SpawnPoint GetFreeSpawnPoint();
    MapData data_;
    std::vector<SpawnPoint> spawnPoints_;
    std::shared_ptr<NavigationMesh> navMesh_;
    std::shared_ptr<Terrain> terrain_;
    std::unique_ptr<Math::Octree> octree_;
};

}
