#include "stdafx.h"
#include "Quest.h"
#include "ScriptManager.h"
#include "Script.h"
#include "Subsystems.h"
#include "DataProvider.h"
#include "Player.h"
#include "NetworkMessage.h"

namespace Game {

void Quest::RegisterLua(kaguya::State& state)
{
    state["Quest"].setClass(kaguya::UserdataMetatable<Quest>()
        .addFunction("GetVarString", &Quest::_LuaGetVarString)
        .addFunction("SetVarString", &Quest::_LuaSetVarString)
        .addFunction("GetVarNumber", &Quest::_LuaGetVarNumber)
        .addFunction("SetVarNumber", &Quest::_LuaSetVarNumber)
    );
}

Quest::Quest(Player& owner) :
    owner_(owner)
{
    InitializeLua();
}

void Quest::InitializeLua()
{
    Lua::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
}

bool Quest::LoadScript(const std::string& fileName)
{
    if (fileName.empty())
        return false;

    auto script = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(fileName);
    if (!script)
        return false;
    if (!script->Execute(luaState_))
        return false;

    if (Lua::IsFunction(luaState_, "onUpdate"))
        functions_ |= FunctionUpdate;
    return true;
}

void Quest::Update(uint32_t timeElapsed)
{
    if (HaveFunction(FunctionUpdate))
        luaState_["onUpdate"](timeElapsed);
}

void Quest::Write(Net::NetworkMessage& message)
{
    // TODO:
    (void)message;
}

std::string Quest::_LuaGetVarString(const std::string& name)
{
    return GetVar(name).GetString();
}

void Quest::_LuaSetVarString(const std::string& name, const std::string& value)
{
    SetVar(name, Utils::Variant(value));
}

float Quest::_LuaGetVarNumber(const std::string& name)
{
    return GetVar(name).GetFloat();
}

void Quest::_LuaSetVarNumber(const std::string& name, float value)
{
    SetVar(name, Utils::Variant(value));
}

const Utils::Variant& Quest::GetVar(const std::string& name) const
{
    auto it = variables_.find(sa::StringHashRt(name.c_str()));
    if (it != variables_.end())
        return (*it).second;
    return Utils::Variant::Empty;
}

void Quest::SetVar(const std::string& name, const Utils::Variant& val)
{
    variables_[sa::StringHashRt(name.c_str())] = val;
}

}
