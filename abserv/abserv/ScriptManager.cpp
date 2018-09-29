#include "stdafx.h"
#include "ScriptManager.h"
#include "Player.h"
#include "Npc.h"
#include <lua.hpp>
#include "DataProvider.h"
#include "Profiler.h"
#include <filesystem>
#include "StringUtils.h"
#include "LuaScript.h"

namespace Game {

#if __cplusplus < 201703L
// C++14
namespace fs = std::experimental::filesystem;
#else
// C++17
namespace fs = std::filesystem;
#endif

void ScriptManager::LuaErrorHandler(int errCode, const char* message)
{
    LOG_ERROR << "Lua Error (" << errCode << "): " << message << std::endl;
}

void ScriptManager::RegisterLuaAll(kaguya::State& state)
{
    state.openlibs();
    state.setErrorHandler(LuaErrorHandler);
#ifdef DEBUG_GAME
    if (!state.gc().isrunning())
    {
        LOG_ERROR << "Lua GC not running" << std::endl;
    }
#endif
    // Some global function
    state["GameTick"] = kaguya::function([]
    {
        return Utils::AbTick();
    });
    state["ServerId"] = kaguya::function([]
    {
        return Application::Instance->GetServerId();
    });
    state["include"] = kaguya::function([&state](const std::string& file)
    {
        auto script = IO::DataProvider::Instance.GetAsset<LuaScript>(file);
        if (script)
            script->Execute(state);
    });

    // Register all used classes
    GameObject::RegisterLua(state);
    Actor::RegisterLua(state);

    Effect::RegisterLua(state);
    Skill::RegisterLua(state);
    SkillBar::RegisterLua(state);

    Player::RegisterLua(state);
    Npc::RegisterLua(state);

    Game::RegisterLua(state);

    auto mainS = IO::DataProvider::Instance.GetAsset<LuaScript>("/scripts/main.lua");
    if (mainS)
        mainS->Execute(state);
}

}
