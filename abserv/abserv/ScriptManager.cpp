#include "stdafx.h"
#include "ScriptManager.h"
#include "Player.h"
#include "Npc.h"
#include <lua.hpp>
#include "DataProvider.h"
#include "Profiler.h"
#if __cplusplus < 201703L
#   if !defined(__clang__) && !defined(__GNUC__)
#       include <filesystem>
#   else
#       include <experimental/filesystem>
#   endif
#else
#   include <filesystem>
#endif
#include "StringUtils.h"
#include "Script.h"
#include "Subsystems.h"
#include "Item.h"
#include "AreaOfEffect.h"
#include "Party.h"
#include "ItemDrop.h"
#include "Random.h"
#include "Party.h"

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
    state.setErrorHandler(LuaErrorHandler);
#ifdef DEBUG_GAME
    if (!state.gc().isrunning())
        LOG_WARNING << "Lua GC not running" << std::endl;
#endif
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
            script->Execute(state);
    });
    state["include_dir"] = kaguya::function([&state](const std::string& dir)
    {
        // directory must start with /
        auto* dataProv = GetSubsystem<IO::DataProvider>();
        using namespace fs;
        const std::string& dataDir = dataProv->GetDataDir();
        size_t dataDirLen = dataDir.length();
        std::string absDir = dataDir + Utils::NormalizeFilename(dir);
        recursive_directory_iterator end_itr;
        try
        {
            for (recursive_directory_iterator itr(absDir); itr != end_itr; ++itr)
            {
                std::string s = itr->path().string();
                std::string ext = itr->path().extension().string();
                if (Utils::StringEquals(ext, ".lua"))
                {
                    std::string _s = Utils::NormalizeFilename(s.substr(dataDirLen));
                    auto script = dataProv->GetAsset<Script>(_s);
                    if (script)
                        script->Execute(state);
                }
            }
        }
        catch (filesystem_error& ex)
        {
            LOG_ERROR << ex.what() << std::endl;
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

    Game::RegisterLua(state);

    // Execute main script with definitions, constants, etc.
    auto mainS = GetSubsystem<IO::DataProvider>()->GetAsset<Script>("/scripts/main.lua");
    if (mainS)
        mainS->Execute(state);
}

void ScriptManager::CollectGarbage(kaguya::State& state)
{
    state.gc().step();
}

}
