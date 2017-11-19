#pragma once

#include "NavigationMesh.h"
#include <string>
#include "IOAsset.h"

namespace IO {

class IONavMesh : public IOAssetImpl<Game::NavigationMesh>
{
public:
    IONavMesh() = default;

    bool Import(Game::NavigationMesh* asset, const std::string& name) override;
};

}

