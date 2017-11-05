#pragma once

#include <memory>

namespace Game {

/// Holds all the map data. Just static data that may be shared by more games.
class Map : public std::enable_shared_from_this<Map>
{
public:
    Map();
    ~Map();
};

}
