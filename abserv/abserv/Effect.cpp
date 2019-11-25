#include "stdafx.h"
#include "Effect.h"
#include "Actor.h"
#include "ScriptManager.h"
#include "DataProvider.h"
#include "Subsystems.h"
#include "Skill.h"
#include "Item.h"

namespace Game {

void Effect::RegisterLua(kaguya::State& state)
{
    state["Effect"].setClass(kaguya::UserdataMetatable<Effect>()
        .addFunction("GetStartTime", &Effect::GetStartTime)
        .addFunction("GetEndTime", &Effect::GetEndTime)
        .addFunction("GetTicks", &Effect::GetTicks)
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
        functions_ |= FunctionUpdate;
    if (Lua::IsFunction(luaState_, "getSkillCost"))
        functions_ |= FunctionGetSkillCost;
    if (Lua::IsFunction(luaState_, "getDamage"))
        functions_ |= FunctionGetDamage;
    if (Lua::IsFunction(luaState_, "getAttackSpeed"))
        functions_ |= FunctionGetAttackSpeed;
    if (Lua::IsFunction(luaState_, "getAttackDamageType"))
        functions_ |= FunctionGetAttackDamageType;
    if (Lua::IsFunction(luaState_, "getAttackDamage"))
        functions_ |= FunctionGetAttackDamage;
    if (Lua::IsFunction(luaState_, "onAttack"))
        functions_ |= FunctionOnAttack;
    if (Lua::IsFunction(luaState_, "onGettingAttacked"))
        functions_ |= FunctionOnGettingAttacked;
    if (Lua::IsFunction(luaState_, "onUseSkill"))
        functions_ |= FunctionOnUseSkill;
    if (Lua::IsFunction(luaState_, "onSkillTargeted"))
        functions_ |= FunctionOnSkillTargeted;
    if (Lua::IsFunction(luaState_, "onAttacked"))
        functions_ |= FunctionOnAttacked;
    if (Lua::IsFunction(luaState_, "onInterruptingAttack"))
        functions_ |= FunctionOnInterruptingAttack;
    if (Lua::IsFunction(luaState_, "onInterruptingSkill"))
        functions_ |= FunctionOnInterruptingSkill;
    if (Lua::IsFunction(luaState_, "onKnockingDown"))
        functions_ |= FunctionOnKnockingDown;
    if (Lua::IsFunction(luaState_, "onHealing"))
        functions_ |= FunctionOnHealing;
    if (Lua::IsFunction(luaState_, "onGetCriticalHit"))
        functions_ |= FunctionOnGetCriticalHit;
    if (Lua::IsFunction(luaState_, "getArmor"))
        functions_ |= FunctionGetArmor;
    if (Lua::IsFunction(luaState_, "getArmorPenetration"))
        functions_ |= FunctionGetArmorPenetration;
    if (Lua::IsFunction(luaState_, "getAttributeValue"))
        functions_ |= FunctionGetAttributeValue;
    if (Lua::IsFunction(luaState_, "getResources"))
        functions_ |= FunctionGetResources;
    if (Lua::IsFunction(luaState_, "getSkillRecharge"))
        functions_ |= FunctionGetSkillRecharge;
    return true;
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
    {
        auto source = source_.lock();
        auto target = target_.lock();
        Lua::CallFunction(luaState_, "onRemove", source.get(), target.get());
    }
    cancelled_ = true;
}

void Effect::GetSkillRecharge(Skill* skill, int32_t& recharge)
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

void Effect::GetAttributeValue(uint32_t index, uint32_t& value)
{
    if (!HaveFunction(FunctionGetAttributeValue))
        return;
    value = luaState_["getAttributeValue"](index, value);
}

void Effect::GetAttackDamage(int32_t& value)
{
    if (!HaveFunction(FunctionGetAttackDamage))
        return;
    value = luaState_["getAttackDamage"](value);
}

void Effect::GetRecources(int & maxHealth, int & maxEnergy)
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

bool Effect::Serialize(IO::PropWriteStream& stream)
{
    stream.Write<uint8_t>(EffectAttrId);
    stream.Write<uint32_t>(data_.index);

    stream.Write<uint8_t>(EffectAttrTicks);
    stream.Write<uint32_t>(ticks_);

    return true;
}

bool Effect::Unserialize(IO::PropReadStream& stream)
{
    uint8_t attr;
    while (stream.Read<uint8_t>(attr) && attr != EffectAttrEnd)
    {
        if (!UnserializeProp(static_cast<EffectAttr>(attr), stream))
            return false;
    }
    return true;
}

bool Effect::UnserializeProp(EffectAttr attr, IO::PropReadStream& stream)
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
