#pragma once

#include "NavigationMesh.h"
#include "Octree.h"
#include "Terrain.h"

namespace Game {

/// Database map data
struct MapData
{
    /// DB ID
    uint32_t id;
    /// The name of the map
    std::string name;
    std::string directory;
};

/// Holds all the map data, spawns NavMesh.
class Map
{
public:
    Map();
    virtual ~Map();

    bool Load();
    void Update(uint32_t delta);
    MapData data_;
    std::shared_ptr<NavigationMesh> navMesh_;
    std::shared_ptr<Terrain> terrain_;
    std::unique_ptr<Math::Octree> octree_;
};

}
