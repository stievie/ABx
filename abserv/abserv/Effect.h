#pragma once

#include "PropStream.h"
#include <forward_list>
#include <AB/Entities/Effect.h>
#include "Script.h"
#include "Damage.h"

namespace Game {

class Actor;
class Skill;
class Item;

enum EffectAttr : uint8_t
{
    EffectAttrId = 1,
    EffectAttrTicks,

    // For serialization
    EffectAttrEnd = 254
};

class Effect
{
private:
    enum Function : uint32_t
    {
        FunctionNone         = 0,
        FunctionUpdate       = 1,
        FunctionGetSkillCost = 1 << 1,
        FunctionGetDamage    = 1 << 2,
        FunctionGetAttackSpeed = 1 << 3,
    };
    kaguya::State luaState_;
    std::shared_ptr<Script> script_;
    std::weak_ptr<Actor> target_;
    std::weak_ptr<Actor> source_;
    bool persistent_;
    uint32_t functions_;
    /// Internal effects are not visible to the player, e.g. Effects from the equipments (+armor from Armor, Shield...).
    bool internal_;
    bool UnserializeProp(EffectAttr attr, IO::PropReadStream& stream);
    void InitializeLua();
    bool HaveFunction(Function func)
    {
        return (functions_ & func) == func;
    }
public:
    static void RegisterLua(kaguya::State& state);

    Effect() = delete;
    explicit Effect(const AB::Entities::Effect& effect) :
        persistent_(false),
        functions_(FunctionNone),
        internal_(false),
        data_(effect),
        startTime_(0),
        endTime_(0),
        ticks_(0),
        ended_(false),
        cancelled_(false)
    {
        InitializeLua();
    }
    // non-copyable
    Effect(const Effect&) = delete;
    Effect& operator=(const Effect&) = delete;
    ~Effect() = default;

    /// Gets saved to the DB when player logs out, e.g. Dishonored.
    bool IsPersistent() const
    {
        return persistent_;
    }
    bool IsInternal() const { return internal_; }

    bool LoadScript(const std::string& fileName);
    void Update(uint32_t timeElapsed);
    bool Start(std::shared_ptr<Actor> source, std::shared_ptr<Actor> target);
    /// Remove Effect before it ends
    void Remove();
    /// Get real cost of a skill
    /// \param skill The Skill
    /// \param activation Activation time
    /// \param energy Energy cost
    /// \param adrenaline Adrenaline cost
    /// \param overcast Causes overcast
    /// \param hp HP scarifies in percent of max health
    void GetSkillCost(Skill* skill,
        int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp);
    /// Get real damage. It may be in-/decreased by some effects.
    void GetDamage(DamageType type, int32_t& value);
    void GetAttackSpeed(Item* weapon, uint32_t& value);

    bool Serialize(IO::PropWriteStream& stream);
    bool Unserialize(IO::PropReadStream& stream);

    AB::Entities::Effect data_;

    int64_t startTime_;
    int64_t endTime_;
    /// Duration
    uint32_t ticks_;
    bool ended_;
    bool cancelled_;

    int64_t GetStartTime() const { return startTime_; }
    int64_t GetEndTime() const { return endTime_; }
    uint32_t GetTicks() const { return ticks_; }
};

/// effects are fist-in-last-out
typedef std::vector<std::shared_ptr<Effect>> EffectList;

enum ConditionType : uint8_t
{
    ConditionNone = 0,
    ConditionDazed,
    ConditionCrackedArmor,
    ConditionBlind,
    ConditionBleeding,
    ConditionBurning,
    ConditionPoison,
    ConditionDesease,
    ConditionWeakness,
    ConditionDeepWound,
    ConditionCrippled
};

}

