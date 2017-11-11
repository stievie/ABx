#include "stdafx.h"
#include "Skill.h"
#include "Creature.h"
#include "GameManager.h"
#include "Utils.h"

namespace Game {

void Skill::RegisterLua(kaguya::State& state)
{
    state["Skill"].setClass(kaguya::UserdataMetatable<Skill>()
        /*        .addFunction("GetName", &Skill::GetName)
        .addFunction("SetName", &Skill::SetName)
        .addFunction("GetDescription", &Skill::GetDescription)
        .addFunction("SetDescription", &Skill::SetDescription)
        .addFunction("GetCooldownTime", &Skill::GetCooldownTime)
        .addFunction("SetCooldownTime", &Skill::SetCooldownTime)*/
    );
}

void Skill::InitializeLua()
{
    GameManager::RegisterLuaAll(luaState_);
}

bool Skill::LoadScript(const std::string& fileName)
{
    if (!luaState_.dofile(fileName.c_str()))
        return false;

    name_ = (const char*)luaState_["name"];
    costEnergy_ = luaState_["costEnergy"];
    costAdrenaline_ = luaState_["costAdrenaline"];
    activation_ = luaState_["activation"];
    recharge_ = luaState_["recharge"];
    overcast_ = luaState_["overcast"];

    return true;
}

void Skill::Update(uint32_t timeElapsed)
{
    AB_UNUSED(timeElapsed);
    if (startUse_ != 0 && startUse_ + activation_ <= Utils::AbTick())
    {
        luaState_["onEndUse"](source_, target_);
        startUse_ = 0;
    }
}

bool Skill::StartUse(Creature* source, Creature* target)
{
    startUse_ = Utils::AbTick();
    source_ = source;
    target_ = target;
    if (!luaState_["onStartUse"](source, target))
    {
        startUse_ = 0;
        return false;
    }
    return true;
}

void Skill::CancelUse()
{
    luaState_["onCancelUse"]();
    startUse_ = 0;
}

}
