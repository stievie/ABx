#pragma once

#include "IOAsset.h"
#include "LuaScript.h"

namespace IO {

class IOLuaScript : public IOAssetImpl<Game::LuaScript>
{
public:
    IOLuaScript() = default;
    bool Import(Game::LuaScript* asset, const std::string& name) override;
};

}
