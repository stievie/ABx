#include "stdafx.h"
#include "AiLoader.h"
#include "DataProvider.h"
#include "Subsystems.h"
#include "Script.h"

namespace AI {

static void LuaErrorHandler(int errCode, const char* message)
{
    LOG_ERROR << "Lua Error (" << errCode << "): " << message << std::endl;
}

bool AiLoader::ExecuteScript(kaguya::State& state, const std::string& file)
{
    // The AI library does not set an error handler
    state.setErrorHandler(LuaErrorHandler);
    auto* dp = GetSubsystem<IO::DataProvider>();
    auto script = dp->GetAsset<Game::Script>(file);
    if (!script)
        return false;
    return script->Execute(state);
}

void AiLoader::LoadError(const std::string& message)
{
    LOG_ERROR << message << std::endl;
}

}
