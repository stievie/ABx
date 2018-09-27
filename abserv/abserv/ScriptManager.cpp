#include "stdafx.h"
#include "ScriptManager.h"
#include "Player.h"
#include "Npc.h"
#include <lua.hpp>
#include "DataProvider.h"
#include "Profiler.h"
#include <filesystem>

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

void ScriptManager::RequireDirectory(kaguya::State& state, const std::string& dir)
{
    LOG_INFO << "ScriptManager::LoadDirectory() " << dir << std::endl;
    using namespace fs;
    recursive_directory_iterator end_itr;

    try
    {
        for (recursive_directory_iterator itr(dir); itr != end_itr; ++itr)
        {
            std::string s = itr->path().string();
            s = (s.size() >= 4 ? s.substr(s.size() - 4) : "");
            if (s == ".lua")
                if (!state.dofile(itr->path().string()))
                    return;
        }
    }
    catch (filesystem_error&)
    {
        return;
    }
}

void ScriptManager::RegisterLuaAll(kaguya::State& state)
{
    AB_PROFILE;
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
    state["RequireDirectory"] = kaguya::function([&state](const std::string& dir)
    {
        RequireDirectory(state, dir);
    });

    // Register all used classes
    GameObject::RegisterLua(state);
    Actor::RegisterLua(state);

    Game::RegisterLua(state);

    Effect::RegisterLua(state);
    Skill::RegisterLua(state);
    SkillBar::RegisterLua(state);

    Player::RegisterLua(state);
    Npc::RegisterLua(state);

    state.dofile(IO::DataProvider::Instance.GetDataFile("/scripts/main.lua"));
}

}
