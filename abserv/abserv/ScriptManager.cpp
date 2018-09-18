#include "stdafx.h"
#include "ScriptManager.h"
#include "Player.h"
#include "Npc.h"
#include <lua.hpp>

namespace Game {

void ScriptManager::LuaErrorHandler(int errCode, const char* message)
{
    LOG_ERROR << "Lua Error (" << errCode << "): " << message << std::endl;
}

void ScriptManager::RegisterLuaAll(kaguya::State& state)
{
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

    // Register all used classes
    GameObject::RegisterLua(state);
    Actor::RegisterLua(state);

    Game::RegisterLua(state);

    Effect::RegisterLua(state);
    Skill::RegisterLua(state);
    SkillBar::RegisterLua(state);

    Player::RegisterLua(state);
    Npc::RegisterLua(state);
}

}
