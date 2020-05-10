/**
 * Copyright 2017-2020 Stefan Ascher
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

#include "stdafx.h"
#include "Effect.h"
#include "Actor.h"
#include "ScriptManager.h"
#include "DataProvider.h"
#include "Skill.h"
#include "Item.h"

namespace Game {

void Effect::RegisterLua(kaguya::State& state)
{
    state["Effect"].setClass(kaguya::UserdataMetatable<Effect>()
        .addFunction("GetStartTime", &Effect::GetStartTime)
        .addFunction("GetEndTime", &Effect::GetEndTime)
        .addFunction("GetTicks", &Effect::GetTicks)
        .addFunction("GetSource", &Effect::_LuaGetSource)
        .addFunction("GetTarget", &Effect::_LuaGetTarget)
        .addFunction("Index", &Effect::GetIndex)
    );
}

void Effect::InitializeLua()
{
    Lua::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
}

bool Effect::LoadScript(const std::string& fileName)
{
    script_ = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(fileName);
    if (!script_)
        return false;
    if (!script_->Execute(luaState_))
        return false;

    persistent_ = luaState_["isPersistent"];
    if (Lua::IsBool(luaState_, "internal"))
        internal_ = luaState_["internal"];

    if (Lua::IsFunction(luaState_, "onUpdate"))
        sa::bits::set(functions_, FunctionUpdate);
    if (Lua::IsFunction(luaState_, "getSkillCost"))
        sa::bits::set(functions_, FunctionGetSkillCost);
    if (Lua::IsFunction(luaState_, "getDamage"))
        sa::bits::set(functions_, FunctionGetDamage);
    if (Lua::IsFunction(luaState_, "getAttackSpeed"))
        sa::bits::set(functions_, FunctionGetAttackSpeed);
    if (Lua::IsFunction(luaState_, "getAttackDamageType"))
        sa::bits::set(functions_, FunctionGetAttackDamageType);
    if (Lua::IsFunction(luaState_, "getAttackDamage"))
        sa::bits::set(functions_, FunctionGetAttackDamage);
    if (Lua::IsFunction(luaState_, "onAttack"))
        sa::bits::set(functions_, FunctionOnAttack);
    if (Lua::IsFunction(luaState_, "onGettingAttacked"))
        sa::bits::set(functions_, FunctionOnGettingAttacked);
    if (Lua::IsFunction(luaState_, "onUseSkill"))
        sa::bits::set(functions_, FunctionOnUseSkill);
    if (Lua::IsFunction(luaState_, "onSkillTargeted"))
        sa::bits::set(functions_, FunctionOnSkillTargeted);
    if (Lua::IsFunction(luaState_, "onAttacked"))
        sa::bits::set(functions_, FunctionOnAttacked);
    if (Lua::IsFunction(luaState_, "onInterruptingAttack"))
        sa::bits::set(functions_, FunctionOnInterruptingAttack);
    if (Lua::IsFunction(luaState_, "onInterruptingSkill"))
        sa::bits::set(functions_, FunctionOnInterruptingSkill);
    if (Lua::IsFunction(luaState_, "onKnockingDown"))
        sa::bits::set(functions_, FunctionOnKnockingDown);
    if (Lua::IsFunction(luaState_, "onHealing"))
        sa::bits::set(functions_, FunctionOnHealing);
    if (Lua::IsFunction(luaState_, "onGetCriticalHit"))
        sa::bits::set(functions_, FunctionOnGetCriticalHit);
    if (Lua::IsFunction(luaState_, "getArmor"))
        sa::bits::set(functions_, FunctionGetArmor);
    if (Lua::IsFunction(luaState_, "getArmorPenetration"))
        sa::bits::set(functions_, FunctionGetArmorPenetration);
    if (Lua::IsFunction(luaState_, "getAttributeRank"))
        sa::bits::set(functions_, FunctionGetAttributeRank);
    if (Lua::IsFunction(luaState_, "getResources"))
        sa::bits::set(functions_, FunctionGetResources);
    if (Lua::IsFunction(luaState_, "getSkillRecharge"))
        sa::bits::set(functions_, FunctionGetSkillRecharge);
    if (Lua::IsFunction(luaState_, "onRemove"))
        sa::bits::set(functions_, FunctionOnRemoved);
    return true;
}

Actor* Effect::_LuaGetTarget()
{
    if (auto a = target_.lock())
        return a.get();
    return nullptr;
}

Actor* Effect::_LuaGetSource()
{
    if (auto a = source_.lock())
        return a.get();
    return nullptr;
}

void Effect::Update(uint32_t timeElapsed)
{
    if (endTime_ == 0)
        return;

    auto source = source_.lock();
    auto target = target_.lock();
    if (HaveFunction(FunctionUpdate))
        luaState_["onUpdate"](source.get(), target.get(), timeElapsed);
    if (endTime_ <= Utils::Tick())
    {
        luaState_["onEnd"](source.get(), target.get());
        ended_ = true;
    }
}

bool Effect::Start(std::shared_ptr<Actor> source, std::shared_ptr<Actor> target, uint32_t time)
{
    target_ = target;
    source_ = source;
    startTime_ = Utils::Tick();
    if (time == 0)
        ticks_ = luaState_["getDuration"](source.get(), target.get());
    else
        ticks_ = time;
    endTime_ = startTime_ + ticks_;
    const bool succ = luaState_["onStart"](source.get(), target.get());
    if (!succ)
        endTime_ = 0;
    return succ;
}

void Effect::Remove()
{
    // The Effect was removed before it ended
    if (HaveFunction(FunctionOnRemoved))
    {
        auto source = source_.lock();
        auto target = target_.lock();
        Lua::CallFunction(luaState_, "onRemove", source.get(), target.get());
    }
    cancelled_ = true;
}

void Effect::GetSkillRecharge(Skill* skill, uint32_t& recharge)
{
    if (!HaveFunction(FunctionGetSkillRecharge))
        return;

    recharge = luaState_["getSkillRecharge"](skill, recharge);
}

void Effect::GetSkillCost(Skill* skill,
    int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp)
{
    if (!HaveFunction(FunctionGetSkillCost))
        return;

    kaguya::tie(activation, energy, adrenaline, overcast, hp) =
        luaState_["getSkillCost"](skill, activation, energy, adrenaline, overcast, hp);
}

void Effect::GetDamage(DamageType type, int32_t& value, bool& critical)
{
    if (!HaveFunction(FunctionGetDamage))
        return;

    kaguya::tie(value, critical) =
        luaState_["getDamage"](static_cast<int>(type), value, critical);
}

void Effect::GetAttackSpeed(Item* weapon, uint32_t& value)
{
    if (!HaveFunction(FunctionGetAttackSpeed))
        return;
    value = luaState_["getAttackSpeed"](weapon, value);
}

void Effect::GetAttackDamageType(DamageType& type)
{
    if (!HaveFunction(FunctionGetAttackDamageType))
        return;
    type = luaState_["getAttackDamageType"](type);
}

void Effect::GetArmor(DamageType type, int& value)
{
    if (!HaveFunction(FunctionGetArmor))
        return;
    value = luaState_["getArmor"](type, value);
}

void Effect::GetArmorPenetration(float& value)
{
    if (!HaveFunction(FunctionGetArmorPenetration))
        return;
    value = luaState_["getArmorPenetration"](value);
}

void Effect::GetAttributeRank(Attribute index, int32_t& value)
{
    if (!HaveFunction(FunctionGetAttributeRank))
        return;
    value = luaState_["getAttributeRank"](static_cast<uint32_t>(index), value);
}

void Effect::GetAttackDamage(int32_t& value)
{
    if (!HaveFunction(FunctionGetAttackDamage))
        return;
    value = luaState_["getAttackDamage"](value);
}

void Effect::GetRecources(int& maxHealth, int& maxEnergy)
{
    if (!HaveFunction(FunctionGetResources))
        return;
    kaguya::tie(maxHealth, maxEnergy) = luaState_["getResources"](maxHealth, maxEnergy);
}

void Effect::OnAttack(Actor* source, Actor* target, bool& value)
{
    if (HaveFunction(FunctionOnAttack))
        value = luaState_["onAttack"](source, target);
}

void Effect::OnAttacked(Actor* source, Actor* target, DamageType type, int32_t damage, bool& success)
{
    if (HaveFunction(FunctionOnAttacked))
        success = luaState_["onAttacked"](source, target, type, damage);
}

void Effect::OnGettingAttacked(Actor* source, Actor* target, bool& value)
{
    if (HaveFunction(FunctionOnGettingAttacked))
        value = luaState_["onGettingAttacked"](source, target);
}

void Effect::OnUseSkill(Actor* source, Actor* target, Skill* skill, bool& value)
{
    if (HaveFunction(FunctionOnUseSkill))
        value = luaState_["onUseSkill"](source, target, skill);
}

void Effect::OnSkillTargeted(Actor* source, Actor* target, Skill* skill, bool& value)
{
    if (HaveFunction(FunctionOnSkillTargeted))
        value = luaState_["onSkillTargeted"](source, target, skill);
}

void Effect::OnInterruptingAttack(bool& value)
{
    if (HaveFunction(FunctionOnInterruptingAttack))
        value = luaState_["onInterruptingAttack"]();
}

void Effect::OnInterruptingSkill(AB::Entities::SkillType type, Skill* skill, bool& value)
{
    if (HaveFunction(FunctionOnInterruptingSkill))
        value = luaState_["onInterruptingSkill"](type, skill);
}

void Effect::OnKnockingDown(Actor* source, Actor* target, uint32_t time, bool& value)
{
    if (HaveFunction(FunctionOnKnockingDown))
        value = luaState_["onKnockingDown"](source, target, time);
}

void Effect::OnGetCriticalHit(Actor* source, Actor* target, bool& value)
{
    if (HaveFunction(FunctionOnKnockingDown))
        value = luaState_["onGetCriticalHit"](source, target);
}

void Effect::OnHealing(Actor* source, Actor* target, int& value)
{
    if (HaveFunction(FunctionOnHealing))
        value = luaState_["onHealing"](source, target, value);
}

bool Effect::Serialize(sa::PropWriteStream& stream)
{
    stream.Write<uint8_t>(EffectAttrId);
    stream.Write<uint32_t>(data_.index);

    stream.Write<uint8_t>(EffectAttrTicks);
    stream.Write<uint32_t>(ticks_);

    return true;
}

bool Effect::Unserialize(sa::PropReadStream& stream)
{
    uint8_t attr;
    while (stream.Read<uint8_t>(attr) && attr != EffectAttrEnd)
    {
        if (!UnserializeProp(static_cast<EffectAttr>(attr), stream))
            return false;
    }
    return true;
}

bool Effect::UnserializeProp(EffectAttr attr, sa::PropReadStream& stream)
{
    switch (attr)
    {
    case EffectAttrId:
        break;
    case EffectAttrTicks:
        return stream.Read<uint32_t>(ticks_);
    default:
        break;
    }
    return false;
}

}
