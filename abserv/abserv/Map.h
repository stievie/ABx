#pragma once

#include "NavigationMesh.h"
#include "Octree.h"
#include "Terrain.h"
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

/// Holds all the map data, static objects, NavMesh.
class Map
{
private:
    std::weak_ptr<Game> game_;
    /// Load objects from Urho3D scene file
    bool LoadObjects();
    void LoadNode(const pugi::xml_node& node);
public:
    Map(std::shared_ptr<Game> game);
    virtual ~Map();

    bool Load();
    void Update(uint32_t delta);
    MapData data_;
    std::shared_ptr<NavigationMesh> navMesh_;
    std::shared_ptr<Terrain> terrain_;
    std::unique_ptr<Math::Octree> octree_;
};

}
