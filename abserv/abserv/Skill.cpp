#include "stdafx.h"
#include "Skill.h"
#include "Creature.h"
#include "ScriptManager.h"

namespace Game {

void Skill::RegisterLua(kaguya::State& state)
{
    state["Skill"].setClass(kaguya::UserdataMetatable<Skill>()
        .addFunction("Disable", &Skill::Disable)
        .addFunction("Interrupt", &Skill::Interrupt)
        .addFunction("GetSource", &Skill::GetSource)
        .addFunction("GetTarget", &Skill::GetTarget)
    );
}

void Skill::InitializeLua()
{
    ScriptManager::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
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
        source_ = nullptr;
        target_ = nullptr;
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
    source_ = nullptr;
    target_ = nullptr;
}

void Skill::Interrupt()
{
    luaState_["onEndUse"](source_, target_);
    startUse_ = 0;
    source_ = nullptr;
    target_ = nullptr;
    // recharged_ remains
}

}
