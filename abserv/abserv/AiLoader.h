#pragma once

#include "Logger.h"
#include "LuaLoader.h"

namespace AI {

class AiLoader : public LuaLoader
{
protected:
    bool ExecuteScript(kaguya::State& state, const std::string& file) override;
    void LoadError(const std::string& message) override;
public:
    explicit AiLoader(Registry& reg) :
        LuaLoader(reg)
    { }
};

}
