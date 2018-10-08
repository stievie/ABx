#pragma once

#include "IOAsset.h"
#include "LuaScript.h"

namespace IO {

/// Loads a Lua script and stores the compiled byte code.
class IOLuaScript : public IOAssetImpl<Game::LuaScript>
{
public:
    bool Import(Game::LuaScript* asset, const std::string& name) override;
};

}
