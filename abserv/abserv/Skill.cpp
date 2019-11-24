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
        .addFunction("Index",        &Skill::GetIndex)   // Hm, GetIndex name throws an exception...
    );
}

void Skill::InitializeLua()
{
    Lua::RegisterLuaAll(luaState_);
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
    if (Lua::IsNumber(luaState_, "hp"))
        hp_ = luaState_["hp"];

    if (Lua::IsNumber(luaState_, "range"))
        range_ = static_cast<Ranges>(luaState_["range"]);
    if (Lua::IsNumber(luaState_, "effect"))
        skillEffect_ = static_cast<uint32_t>(luaState_["effect"]);
    if (Lua::IsNumber(luaState_, "effectTarget"))
        effectTarget_ = static_cast<uint32_t>(luaState_["effectTarget"]);
    if (Lua::IsNumber(luaState_, "canInterrupt"))
        canInterrupt_ = luaState_["canInterrupt"];

    haveOnCancelled_ = Lua::IsFunction(luaState_, "onCancelled");
    haveOnInterrupted_ = Lua::IsFunction(luaState_, "onInterrupted");

    return true;
}

void Skill::Update(uint32_t timeElapsed)
{
    AB_UNUSED(timeElapsed);
    if (startUse_ != 0)
    {
        if (startUse_ + realActivation_ <= Utils::Tick())
        {
            recharged_ = Utils::Tick() + recharge_;
            auto source = source_.lock();
            auto target = target_.lock();
            // A Skill may even fail here, e.g. when resurrecting an already resurrected target
            lastError_ = luaState_["onSuccess"](source.get(), target.get());
            startUse_ = 0;
            if (lastError_ != AB::GameProtocol::SkillErrorNone)
                recharged_ = 0;
            else
            {
                // On success sacrifice the HP
                source->resourceComp_->SetHealth(Components::SetValueType::DecreasePercent, realHp_);
                lastUse_ = Utils::Tick();
            }
            if (source)
                source->CallEvent<void(Skill*)>(EVENT_ON_ENDUSESKILL, this);
            source_.reset();
            target_.reset();
        }
    }
}

bool Skill::CanUseSkill(Actor& source, Actor* target)
{
    bool success = true;
    source.CallEvent<void(Actor*, Skill*, bool&)>(EVENT_ON_USESKILL, target, this, success);
    if (!success)
    {
        lastError_ = AB::GameProtocol::SkillErrorCannotUseSkill;
        return false;
    }
    if (HasTarget(SkillTargetTarget))
    {
        if (!target)
        {
            lastError_ = AB::GameProtocol::SkillErrorInvalidTarget;
            return false;
        }
        if (target->IsUndestroyable())
        {
            lastError_ = AB::GameProtocol::SkillErrorTargetUndestroyable;
            return false;
        }
        bool targetSuccess = true;
        target->CallEvent<void(Actor*, Skill*, bool&)>(EVENT_ON_SKILLTARGETED, &source, this, targetSuccess);
        if (!targetSuccess)
        {
            lastError_ = AB::GameProtocol::SkillErrorInvalidTarget;
            return false;
        }
    }

    if (IsUsing() || !IsRecharged())
        lastError_ = AB::GameProtocol::SkillErrorRecharging;
    return (lastError_ == AB::GameProtocol::SkillErrorNone);
}

int32_t Skill::GetActivation(Actor& source, int32_t activation)
{
    const uint32_t fastcast = source.GetAttributeValue(static_cast<uint32_t>(AttributeIndices::FastCast));
    if (fastcast == 0)
        return activation;
    if (IsType(AB::Entities::SkillTypeSignet))
        return static_cast<int32_t>(static_cast<float>(activation) * (1.0f - (0.03f * static_cast<float>(fastcast))));
    if (IsType(AB::Entities::SkillTypeSpell))
        return static_cast<int32_t>(static_cast<float>(activation) * powf(0.5f, static_cast<float>(fastcast) / 15.0f));
    return activation;
}

AB::GameProtocol::SkillError Skill::StartUse(std::shared_ptr<Actor> source, std::shared_ptr<Actor> target)
{
    lastError_ = AB::GameProtocol::SkillErrorNone;
    if (!CanUseSkill(*source, target.get()))
        return lastError_;

    realEnergy_ = energy_;
    realAdrenaline_ = adrenaline_;
    realActivation_ = GetActivation(*source, activation_);
    realOvercast_ = overcast_;
    realHp_ = hp_;
    // Get real skill cost, which depends on the effects and equipment of the source
    source->inventoryComp_->GetSkillCost(this, realActivation_, realEnergy_, realAdrenaline_, realOvercast_, realHp_);
    source->effectsComp_->GetSkillCost(this, realActivation_, realEnergy_, realAdrenaline_, realOvercast_, realHp_);

    if (source->resourceComp_->GetEnergy() < realEnergy_)
        lastError_ = AB::GameProtocol::SkillErrorNoEnergy;
    if (lastError_ != AB::GameProtocol::SkillErrorNone)
        return lastError_;
    if (source->resourceComp_->GetAdrenaline() < realAdrenaline_)
        lastError_ = AB::GameProtocol::SkillErrorNoAdrenaline;
    if (lastError_ != AB::GameProtocol::SkillErrorNone)
        return lastError_;

    startUse_ = Utils::Tick();

    source_ = source;
    target_ = target;

    lastError_ = luaState_["onStartUse"](source.get(), target.get());
    if (lastError_ != AB::GameProtocol::SkillErrorNone)
    {
        startUse_ = 0;
        recharged_ = 0;
        source_.reset();
        target_.reset();
        return lastError_;
    }
    source->resourceComp_->SetEnergy(Components::SetValueType::Decrease, realEnergy_);
    source->resourceComp_->SetAdrenaline(Components::SetValueType::Decrease, realAdrenaline_);
    source->resourceComp_->SetOvercast(Components::SetValueType::Increase, realOvercast_);
    source->CallEvent<void(Skill*)>(EVENT_ON_STARTUSESKILL, this);
    return lastError_;
}

void Skill::CancelUse()
{
    auto source = source_.lock();
    if (haveOnCancelled_)
    {
        auto target = target_.lock();
        Lua::CallFunction(luaState_, "onCancelled",
            source.get(), target.get());
    }
    if (source)
        source->CallEvent<void(Skill*)>(EVENT_ON_ENDUSESKILL, this);
    startUse_ = 0;
    // No recharging when canceled
    recharged_ = 0;
    source_.reset();
    target_.reset();
}

bool Skill::Interrupt()
{
    if (!IsUsing() || !IsChangingState())
        return false;

    auto source = source_.lock();
    if (haveOnInterrupted_)
    {
        auto target = target_.lock();
        Lua::CallFunction(luaState_, "onInterrupted",
            source.get(), target.get());
    }
    if (source)
    {
        source->CallEvent<void(Skill*)>(EVENT_ON_INTERRUPTEDSKILL, this);
        source->CallEvent<void(Skill*)>(EVENT_ON_ENDUSESKILL, this);
    }
    startUse_ = 0;
    source_.reset();
    target_.reset();
    // recharged_ remains
    return true;
}

float Skill::CalculateCost(const std::function<float(CostType)>& importanceCallback) const
{
    float result = 0.0f;
    if (activation_ > 0)
        result += importanceCallback(CostType::Activation) * activation_;
    if (energy_ > 0)
        result += importanceCallback(CostType::Energy) * energy_;
    if (adrenaline_ > 0)
        result += importanceCallback(CostType::Adrenaline) * adrenaline_;
    if (hp_ > 0)
        result += importanceCallback(CostType::HpSacrify) * hp_;
    if (overcast_ > 0)
        result += importanceCallback(CostType::HpSacrify) * overcast_;

    return result;
}

bool Skill::IsInRange(const Actor* target) const
{
    if (!target)
        return false;
    if (auto s = source_.lock())
    {
        if (s->id_ == target->id_)
            // Self is always in range
            return true;
        return s->IsInRange(range_, target);
    }
    return false;
}

void Skill::AddRecharge(int32_t ms)
{
    recharge_ += ms;
}

}
