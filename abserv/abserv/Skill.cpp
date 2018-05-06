#include "stdafx.h"
#include "Skill.h"
#include "Creature.h"
#include "GameManager.h"

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
    luaState_["this"] = this;
}

bool Skill::LoadScript(const std::string& fileName)
{
    if (!luaState_.dofile(fileName.c_str()))
        return false;

    energy_ = luaState_["costEnergy"];
    adrenaline_ = luaState_["costAdrenaline"];
    activation_ = luaState_["activation"];
    recharge_ = luaState_["recharge"];
    overcast_ = luaState_["overcast"];
    type_ = static_cast<SkillType>(data_.type);

    return true;
}

void Skill::Update(uint32_t timeElapsed)
{
    AB_UNUSED(timeElapsed);
    if (startUse_ != 0 && startUse_ + activation_ <= Utils::AbTick())
    {
        recharged_ = Utils::AbTick() + recharge_;
        luaState_["onEndUse"](source_, target_);
        startUse_ = 0;
    }
}

bool Skill::StartUse(Creature* source, Creature* target)
{
    if (!IsUsing() ||
        !IsRecharged() ||
        source->energy_ < energy_ ||
        source->adrenaline_ < adrenaline_)
        return false;

    startUse_ = Utils::AbTick();

    source_ = source;
    target_ = target;

    if (!luaState_["onStartUse"](source, target))
    {
        startUse_ = 0;
        recharged_ = 0;
        return false;
    }
    source->energy_ += energy_;
    source->adrenaline_ += adrenaline_;
    source->overcast_ += overcast_;
    return true;
}

void Skill::CancelUse()
{
    luaState_["onCancelUse"]();
    startUse_ = 0;
    recharged_ = 0;
}

}
