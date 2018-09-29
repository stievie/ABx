#pragma once

#include "Asset.h"

namespace Game {

class LuaScript : public IO::AssetImpl<LuaScript>
{
private:
    std::vector<char> buffer_;
public:
    LuaScript() = default;
    ~LuaScript() override
    { }

    std::vector<char>& GetBuffer()
    {
        return buffer_;
    }

    /// Execute the script
    bool Execute(kaguya::State& luaState);
};

}
