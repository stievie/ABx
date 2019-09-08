#pragma once

#include "Model.h"
#include "IOAsset.h"

namespace IO {

class IOModel : public IOAssetImpl<Game::Model>
{
public:
    /// Import 3D Model file exported with the import program
    bool Import(Game::Model& asset, const std::string& name) override;
};

}
