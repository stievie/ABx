#pragma once

#include "Asset.h"

namespace Game {

class Script : public IO::Asset
{
private:
    std::vector<char> buffer_;
public:
    Script() :
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
