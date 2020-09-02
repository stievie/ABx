/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "ScriptHelper.h"
#include "Game.h"
#include "GameObject.h"
#include "Player.h"
#include <abscommon/Subsystems.h>
#include <abscommon/SimpleConfigManager.h>
#include <abscommon/Logger.h>
#include <abscommon/StringUtils.h>
#include <abscommon/FileUtils.h>
#include <sa/StringTempl.h>
#include <sa/time.h>
#include <abscommon/Random.h>

static void LuaErrorHandler(int errCode, const char* message)
{
    LOG_ERROR << "Lua Error (" << errCode << "): " << message << std::endl;
}

void InitLuaState(kaguya::State& state)
{
    state.setErrorHandler(LuaErrorHandler);
    state["Tick"] = kaguya::function([]
    {
        return sa::time::tick();
    });
    state["Random"] = kaguya::overload(
        [] { return GetSubsystem<Crypto::Random>()->GetFloat(); },
        [](float max) { return GetSubsystem<Crypto::Random>()->Get<float>(0.0f, max); },
        [](float min, float max) { return GetSubsystem<Crypto::Random>()->Get<float>(min, max); }
    );
    state["include"] = kaguya::function([&state](const std::string& file)
    {
        auto script = GetDataFile(file);
        if (Utils::FileExists(script))
        {
            // Make something like an include guard
            std::string ident(file);
            sa::MakeIdent(ident);
            ident = "__included_" + ident + "__";
            if (IsBool(state, ident))
                return;
            if (state.dofile(script.c_str()))
                state[ident] = true;
        }
    });
    Game::RegisterLua(state);
    GameObject::RegisterLua(state);
    Player::RegisterLua(state);
}

std::string GetDataDir()
{
    auto* cfg = GetSubsystem<IO::SimpleConfigManager>();
    return cfg->GetGlobalString("data_dir", "");
}

std::string GetDataFile(const std::string& name)
{
    return Utils::ConcatPath(GetDataDir(), name);
}
