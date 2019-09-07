#include "stdafx.h"
#include "ScriptManager.h"
#include "Player.h"
#include "Npc.h"
#include "DataProvider.h"
#include "Profiler.h"
#include "StringUtils.h"
#include "Script.h"
#include "Subsystems.h"
#include "Item.h"
#include "AreaOfEffect.h"
#include "Party.h"
#include "ItemDrop.h"
#include "Random.h"
#include "Party.h"
#include "Projectile.h"

namespace Game {

static const char* MAIN_SCRIPT = "/scripts/main.lua";

void ScriptManager::LuaErrorHandler(int errCode, const char* message)
{
    LOG_ERROR << "Lua Error (" << errCode << "): " << message << std::endl;
}

void ScriptManager::RegisterLuaAll(kaguya::State& state)
{
    state.setErrorHandler(LuaErrorHandler);

    // Some global function
    state["Tick"] = kaguya::function([]
    {
        return Utils::Tick();
    });
    state["Random"] = kaguya::function([]
    {
        return GetSubsystem<Crypto::Random>()->GetFloat();
    });
    state["ServerId"] = kaguya::function([]
    {
        return Application::Instance->GetServerId();
    });
    state["NewGroupId"] = kaguya::function([]
    {
        return Party::GetNewId();
    });
    state["include"] = kaguya::function([&state](const std::string& file)
    {
        auto script = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(file);
        if (script)
        {
            // Make something like an include guard
            std::string ident(file);
            Utils::MakeIdent(ident);
            ident = "__included_" + ident + "__";
            if (IsBool(state, ident))
                return;
            script->Execute(state);
            state[ident] = true;
        }
    });

    // Register all used classes
    GameObject::RegisterLua(state);
    Actor::RegisterLua(state);
    Item::RegisterLua(state);
    ItemDrop::RegisterLua(state);

    Effect::RegisterLua(state);
    Skill::RegisterLua(state);
    SkillBar::RegisterLua(state);
    Party::RegisterLua(state);

    AreaOfEffect::RegisterLua(state);
    Player::RegisterLua(state);
    Npc::RegisterLua(state);
    Projectile::RegisterLua(state);

    Game::RegisterLua(state);

    // Execute main script with definitions, constants, etc.
    auto mainS = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(MAIN_SCRIPT);
    if (mainS)
        mainS->Execute(state);
}

void ScriptManager::CollectGarbage(kaguya::State& state)
{
    state.gc().step();
}

}
