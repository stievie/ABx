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
        .addFunction("GetName",      &Skill::_LuaGetName)
        .addFunction("Disable",      &Skill::Disable)
        .addFunction("Interrupt",    &Skill::Interrupt)
        .addFunction("GetSource",    &Skill::GetSource)
        .addFunction("GetTarget",    &Skill::GetTarget)
        .addFunction("AddRecharge",  &Skill::AddRecharge)
        .addFunction("SetRecharged", &Skill::SetRecharged)
        .addFunction("IsUsing",      &Skill::IsUsing)
        .addFunction("IsRecharged",  &Skill::IsRecharged)
        .addFunction("GetType",      &Skill::_LuaGetType)
        .addFunction("GetIndex",     &Skill::_LuaGetIndex)
        .addFunction("IsElite",      &Skill::_LuaIsElite)
        .addFunction("IsInRange",    &Skill::IsInRange)
        .addFunction("HasEffect",    &Skill::HasEffect)
        .addFunction("HasTarget",    &Skill::HasTarget)
        .addFunction("IsType",       &Skill::IsType)
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
    haveOnCancelled_ = ScriptManager::IsFunction(luaState_, "onCancelled");
    haveOnInterrupted_ = ScriptManager::IsFunction(luaState_, "onInterrupted");

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
            auto source = source_.lock();
            auto target = target_.lock();
            // A Skill may even fail here, e.g. when resurrecting an already resurrected target
            lastError_ = luaState_["onSuccess"](source ? source.get() : nullptr, target ? target.get() : nullptr);
            startUse_ = 0;
            if (lastError_ != AB::GameProtocol::SkillErrorNone)
                recharged_ = 0;
            if (source)
                source->OnEndUseSkill(this);
            source_.reset();
            target_.reset();
        }
    }
}

AB::GameProtocol::SkillError Skill::StartUse(std::shared_ptr<Actor> source, std::shared_ptr<Actor> target)
{
    lastError_ = AB::GameProtocol::SkillErrorNone;
    if (IsUsing() || !IsRecharged())
        lastError_ = AB::GameProtocol::SkillErrorRecharging;
    if (lastError_ != AB::GameProtocol::SkillErrorNone)
        return lastError_;

    realEnergy_ = energy_;
    realAdrenaline_ = adrenaline_;
    realActivation_ = activation_;
    realOvercast_ = overcast_;
    // Get real skill cost, which depends on the effects of the source (e.g. equipment)
    source->effectsComp_.GetSkillCost(this, realEnergy_, realAdrenaline_, realActivation_, realOvercast_);

    if (source->resourceComp_.GetEnergy() < realEnergy_)
        lastError_ = AB::GameProtocol::SkillErrorNoEnergy;
    if (lastError_ != AB::GameProtocol::SkillErrorNone)
        return lastError_;
    if (source->resourceComp_.GetAdrenaline() < realAdrenaline_)
        lastError_ = AB::GameProtocol::SkillErrorNoAdrenaline;
    if (lastError_ != AB::GameProtocol::SkillErrorNone)
        return lastError_;

    startUse_ = Utils::AbTick();

    source_ = source;
    target_ = target;

    lastError_ = luaState_["onStartUse"](source ? source.get() : nullptr, target ? target.get() : nullptr);
    if (lastError_ != AB::GameProtocol::SkillErrorNone)
    {
        startUse_ = 0;
        recharged_ = 0;
        source_.reset();
        target_.reset();
        return lastError_;
    }
    source->resourceComp_.SetEnergy(Components::SetValueType::Decrease, realEnergy_);
    source->resourceComp_.SetAdrenaline(Components::SetValueType::Decrease, realAdrenaline_);
    source->resourceComp_.SetOvercast(Components::SetValueType::Increase, realOvercast_);
    source->OnStartUseSkill(this);
    return lastError_;
}

void Skill::CancelUse()
{
    auto source = source_.lock();
    if (haveOnCancelled_)
    {
        auto target = target_.lock();
        ScriptManager::CallFunction(luaState_, "onCancelled",
            source ? source.get() : nullptr, target ? target.get() : nullptr);
    }
    if (source)
        source->OnEndUseSkill(this);
    startUse_ = 0;
    // No recharging when canceled
    recharged_ = 0;
    source_.reset();
    target_.reset();
}

void Skill::Interrupt()
{
    auto source = source_.lock();
    if (haveOnInterrupted_)
    {
        auto target = target_.lock();
        ScriptManager::CallFunction(luaState_, "onInterrupted",
            source ? source.get() : nullptr, target ? target.get() : nullptr);
    }
    if (source)
        source->OnEndUseSkill(this);
    startUse_ = 0;
    source_.reset();
    target_.reset();
    // recharged_ remains
}

bool Skill::IsInRange(Actor* target)
{
    if (!target)
        return false;
    if (auto s = source_.lock())
    {
        return s->IsInRange(range_, target);
    }
    return false;
}

void Skill::AddRecharge(int16_t ms)
{
    recharge_ += ms;
}

}
