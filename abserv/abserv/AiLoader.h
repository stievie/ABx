#pragma once

#include "Logger.h"
#include "Loader.h"

namespace AI {

class AiLoader : public Loader
{
protected:
    bool ExecuteScript(kaguya::State& state, const std::string& file) override;
    void LoadError(const std::string& message) override;
public:
    explicit AiLoader(Registry& reg) :
        Loader(reg)
    { }
};

}
