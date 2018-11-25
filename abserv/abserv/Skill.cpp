#include "stdafx.h"
#include "Skill.h"
#include "Actor.h"
#include "ScriptManager.h"
#include "DataProvider.h"
#include "Subsystems.h"
#include "ResourceComp.h"

namespace Game {

void Skill::RegisterLua(kaguya::State& state)
{
    state["Skill"].setClass(kaguya::UserdataMetatable<Skill>()
        .addFunction("GetName", &Skill::_LuaGetName)
        .addFunction("Disable", &Skill::Disable)
        .addFunction("Interrupt", &Skill::Interrupt)
        .addFunction("GetSource", &Skill::GetSource)
        .addFunction("GetTarget", &Skill::GetTarget)
        .addFunction("AddRecharge", &Skill::AddRecharge)
        .addFunction("IsUsing", &Skill::IsUsing)
        .addFunction("IsRecharged", &Skill::IsRecharged)
        .addFunction("GetType", &Skill::_LuaGetType)
        .addFunction("GetIndex", &Skill::_LuaGetIndex)
        .addFunction("IsElite", &Skill::_LuaIsElite)
        .addFunction("IsInRange", &Skill::IsInRange)
        .addFunction("HasEffect", &Skill::HasEffect)
        .addFunction("HasTarget", &Skill::HasTarget)
    );
}

void Skill::InitializeLua()
{
    ScriptManager::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
}

bool Skill::LoadScript(const std::string& fileName)
{
    script_ = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(fileName);
    if (!script_)
        return false;
    if (!script_->Execute(luaState_))
        return false;

    energy_ = luaState_["costEnergy"];
    adrenaline_ = luaState_["costAdrenaline"];
    activation_ = luaState_["activation"];
    recharge_ = luaState_["recharge"];
    overcast_ = luaState_["overcast"];
    if (ScriptManager::IsNumber(luaState_, "range"))
        range_ = static_cast<Ranges>(luaState_["range"]);
    if (ScriptManager::IsNumber(luaState_, "effect"))
        skillEffect_ = luaState_["effect"];

    return true;
}

void Skill::Update(uint32_t timeElapsed)
{
    AB_UNUSED(timeElapsed);
    if (startUse_ != 0)
    {
        if (startUse_ + activation_ <= Utils::AbTick())
        {
            recharged_ = Utils::AbTick() + recharge_;
            luaState_["onEndUse"](source_, target_);
            startUse_ = 0;
            source_->OnEndUseSkill();
            source_ = nullptr;
            target_ = nullptr;
        }
    }
}

bool Skill::StartUse(Actor* source, Actor* target)
{
    if (IsUsing() ||
        !IsRecharged() ||
        source->resourceComp_.GetEnergy() < energy_ ||
        source->resourceComp_.GetAdrenaline() < adrenaline_)
        return false;

    startUse_ = Utils::AbTick();

    source_ = source;
    target_ = target;

    if (!luaState_["onStartUse"](source, target))
    {
        startUse_ = 0;
        recharged_ = 0;
        source_ = nullptr;
        target_ = nullptr;
        return false;
    }
    source->resourceComp_.SetEnergy(Components::SetValueType::Decrease, energy_);
    source->resourceComp_.SetAdrenaline(Components::SetValueType::Decrease, adrenaline_);
    source->resourceComp_.SetOvercast(Components::SetValueType::Increase, overcast_);
    source->OnStartUseSkill(this);
    return true;
}

void Skill::CancelUse()
{
    luaState_["onCancelUse"]();
    source_->OnEndUseSkill();
    startUse_ = 0;
    // No recharging when canceled
    recharged_ = 0;
    source_ = nullptr;
    target_ = nullptr;
}

void Skill::Interrupt()
{
    luaState_["onEndUse"](source_, target_);
    source_->OnEndUseSkill();
    startUse_ = 0;
    source_ = nullptr;
    target_ = nullptr;
    // recharged_ remains
}

bool Skill::IsInRange(Actor* target)
{
    if (!source_ || !target)
        return false;
    return source_->IsInRange(range_, target);
}

void Skill::AddRecharge(int16_t ms)
{
    recharge_ += ms;
}

}
