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

#include "LuaSkill.h"
#include <abscommon/StringUtils.h>
#include <abscommon/FileUtils.h>
#include <sa/StringTempl.h>
#include <iostream>

static void InitState(kaguya::State& state)
{
    state["include"] = kaguya::function([&state](const std::string& file)
    {
        extern std::string gDataDir;
        std::string scriptFile = Utils::ConcatPath(gDataDir, file);
        if (!Utils::FileExists(scriptFile))
            return;

        // Make something like an include guard
        std::string ident(file);
        sa::MakeIdent(ident);
        ident = "__included_" + ident + "__";
        if (state[ident].type() == LUA_TBOOLEAN)
            return;
        if (!state.dofile(scriptFile.c_str()))
        {
            std::cerr << lua_tostring(state.state(), -1) << std::endl;
            return;
        }
        state[ident] = true;
    });
}

LuaSkill::LuaSkill()
{
    InitState(state_);
}

bool LuaSkill::Execute(const std::string& script)
{
    if (!state_.dofile(script.c_str()))
    {
        std::cerr << lua_tostring(state_.state(), -1) << std::endl;
        return false;
    }
    return true;
}

int32_t LuaSkill::GetEnergy()
{
    return state_["costEnergy"];
}

int32_t LuaSkill::GetEnergyRegen()
{
    if (state_["costEnergyRegen"].type() == LUA_TNUMBER)
        return state_["costEnergyRegen"];
    return 0;
}

int32_t LuaSkill::GetAdrenaline()
{
    return state_["costAdrenaline"];
}

int32_t LuaSkill::GetActivation()
{
    return state_["activation"];
}

int32_t LuaSkill::GetOvercast()
{
    return state_["overcast"];
}

int32_t LuaSkill::GetHp()
{
    if (state_["hp"].type() == LUA_TNUMBER)
        return state_["hp"];
    return 0;
}

int32_t LuaSkill::GetRecharge()
{
    return state_["recharge"];
}
