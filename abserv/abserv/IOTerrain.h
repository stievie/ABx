#pragma once

#include "IOAsset.h"
#include "Terrain.h"

namespace IO {

class IOTerrain : public IOAssetImpl<Game::Terrain>
{
public:
    IOTerrain() = default;

    bool Import(Game::Terrain* asset, const std::string& name) override;
};

}
