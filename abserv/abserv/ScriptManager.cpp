#include "stdafx.h"
#include "ScriptManager.h"
#include "Player.h"
#include "Npc.h"
#include <lua.hpp>
#include "DataProvider.h"
#include "Profiler.h"
#include <filesystem>
#include "StringUtils.h"
#include "Script.h"
#include "Subsystems.h"

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
        auto script = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(file);
        if (script)
            script->Execute(state);
    });
    state["include_dir"] = kaguya::function([&state](const std::string& dir)
    {
        // directory must start with /
        auto dataProv = GetSubsystem<IO::DataProvider>();
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

    Effect::RegisterLua(state);
    Skill::RegisterLua(state);
    SkillBar::RegisterLua(state);

    Player::RegisterLua(state);
    Npc::RegisterLua(state);

    Game::RegisterLua(state);

    // Execute main script with definitions, constants, etc.
    auto mainS = GetSubsystem<IO::DataProvider>()->GetAsset<Script>("/scripts/main.lua");
    if (mainS)
        mainS->Execute(state);
}

}
