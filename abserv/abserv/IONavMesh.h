#pragma once

#include "NavigationMesh.h"
#include "IOAsset.h"

namespace IO {

class IONavMesh : public IOAssetImpl<Navigation::NavigationMesh>
{
public:
    IONavMesh() = default;

    bool Import(Navigation::NavigationMesh* asset, const std::string& name) override;
};

}

