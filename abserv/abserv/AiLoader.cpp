#include "stdafx.h"
#include "AiLoader.h"
#include "DataProvider.h"
#include "Subsystems.h"
#include "Script.h"

namespace AI {

bool AiLoader::ExecuteScript(kaguya::State& state, const std::string& file)
{
    auto* dp = GetSubsystem<IO::DataProvider>();
    auto script = dp->GetAsset<Game::Script>(file);
    if (!script)
        return false;
    return script->Execute(state);
}

}
