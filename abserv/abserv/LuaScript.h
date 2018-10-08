#pragma once

#include "Asset.h"

namespace Game {

class LuaScript : public IO::Asset
{
private:
    std::vector<char> buffer_;
public:
    LuaScript() :
        IO::Asset()
    { }

    std::vector<char>& GetBuffer()
    {
        return buffer_;
    }

    /// Execute the script
    bool Execute(kaguya::State& luaState);
};

}
