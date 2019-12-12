#include "stdafx.h"
#include "ScriptManager.h"
#include "AreaOfEffect.h"
#include "Crowd.h"
#include "DataProvider.h"
#include "Group.h"
#include "Item.h"
#include "ItemDrop.h"
#include "Npc.h"
#include "Party.h"
#include "Player.h"
#include "Profiler.h"
#include "Projectile.h"
#include "Quest.h"
#include "Random.h"
#include "Script.h"
#include "Subsystems.h"
#include <sa/StringTempl.h>

namespace Game {
namespace Lua {

static const char* MAIN_SCRIPT = "/scripts/main.lua";

static void LuaErrorHandler(int errCode, const char* message)
{
    LOG_ERROR << "Lua Error (" << errCode << "): " << message << std::endl;
}

void RegisterLuaAll(kaguya::State& state)
{
    state.setErrorHandler(LuaErrorHandler);

    // Some global function
    state["Tick"] = kaguya::function([]
    {
        return Utils::Tick();
    });
    state["Random"] = kaguya::overload(
        [] { return GetSubsystem<Crypto::Random>()->GetFloat(); },
        [] (float max){ return GetSubsystem<Crypto::Random>()->Get<float>(0.0f, max); },
        [] (float min, float max){ return GetSubsystem<Crypto::Random>()->Get<float>(min, max); }
    );
    state["ServerId"] = kaguya::function([]
    {
        return Application::Instance->GetServerId();
    });
    state["NewGroupId"] = kaguya::function([]
    {
        return Group::GetNewId();
    });
    state["include"] = kaguya::function([&state](const std::string& file)
    {
        auto script = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(file);
        if (script)
        {
            // Make something like an include guard
            std::string ident(file);
            sa::MakeIdent(ident);
            ident = "__included_" + ident + "__";
            if (IsBool(state, ident))
                return;
            if (script->Execute(state))
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
    Group::RegisterLua(state);
    Party::RegisterLua(state);
    Crowd::RegisterLua(state);
    Quest::RegisterLua(state);

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

void CollectGarbage(kaguya::State& state)
{
    state.gc().step();
}

}
}
