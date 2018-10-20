#pragma once

#include "IOAsset.h"
#include "Script.h"

namespace IO {

/// Loads a Lua script and stores the compiled byte code.
class IOScript : public IOAssetImpl<Game::Script>
{
public:
    bool Import(Game::Script* asset, const std::string& name) override;
};

}
