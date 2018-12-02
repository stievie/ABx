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
        .addFunction("SetRecharged", &Skill::SetRecharged)
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
        skillEffect_ = static_cast<SkillEffect>(luaState_["effect"]);
    if (ScriptManager::IsNumber(luaState_, "effectTarget"))
        effectTarget = static_cast<SkillTarget>(luaState_["effectTarget"]);

    return true;
}

void Skill::Update(uint32_t timeElapsed)
{
    AB_UNUSED(timeElapsed);
    if (startUse_ != 0)
    {
        if (startUse_ + realActivation_ <= Utils::AbTick())
        {
            recharged_ = Utils::AbTick() + recharge_;
            luaState_["onEndUse"](source_, target_);
            startUse_ = 0;
            source_->OnEndUseSkill(this);
            source_ = nullptr;
            target_ = nullptr;
        }
    }
}

AB::GameProtocol::SkillError Skill::StartUse(Actor* source, Actor* target)
{
    if (IsUsing() || !IsRecharged())
        return AB::GameProtocol::SkillErrorRecharging;

    // TODO:
    realEnergy_ = energy_;
    realAdrenaline_ = adrenaline_;
    realActivation_ = activation_;
    realOvercast_ = overcast_;

    if (source->resourceComp_.GetEnergy() < realEnergy_)
        return AB::GameProtocol::SkillErrorNoEnergy;
    if (source->resourceComp_.GetAdrenaline() < realAdrenaline_)
        return AB::GameProtocol::SkillErrorNoAdrenaline;

    startUse_ = Utils::AbTick();

    source_ = source;
    target_ = target;

    AB::GameProtocol::SkillError err = luaState_["onStartUse"](source, target);
    if (err != AB::GameProtocol::SkillErrorNone)
    {
        startUse_ = 0;
        recharged_ = 0;
        source_ = nullptr;
        target_ = nullptr;
        return err;
    }
    source->resourceComp_.SetEnergy(Components::SetValueType::Decrease, realEnergy_);
    source->resourceComp_.SetAdrenaline(Components::SetValueType::Decrease, realAdrenaline_);
    source->resourceComp_.SetOvercast(Components::SetValueType::Increase, realOvercast_);
    source->OnStartUseSkill(this);
    return AB::GameProtocol::SkillErrorNone;
}

void Skill::CancelUse()
{
    luaState_["onCancelUse"]();
    source_->OnEndUseSkill(this);
    startUse_ = 0;
    // No recharging when canceled
    recharged_ = 0;
    source_ = nullptr;
    target_ = nullptr;
}

void Skill::Interrupt()
{
    luaState_["onEndUse"](source_, target_);
    source_->OnEndUseSkill(this);
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
