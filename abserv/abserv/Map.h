#pragma once

#include <memory>
#include <string>
#include "Asset.h"

namespace Game {

/// Database map data
struct MapData
{
    /// DB ID
    uint32_t id;
    /// The name of the map
    std::string name;
};

/// Holds all the map data. Just static data that may be shared by more games.
class Map : public IO::AssetImpl<Map>
{
public:
    Map();
    ~Map();

    MapData data_;
};

}
