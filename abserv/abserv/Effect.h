#pragma once

#include "PropStream.h"
#include <forward_list>
#include <memory>
#include <stdint.h>
#pragma warning(push)
#pragma warning(disable: 4702 4127)
#include <kaguya/kaguya.hpp>
#pragma warning(pop)

namespace Game {

class Creature;

enum EffectAttr : uint8_t
{
    EffectAttrId = 1,
    EffectAttrTicks,

    // For serialization
    EffectAttrEnd = 254
};

enum EffectType : uint8_t
{
    EffectNone = 0,
    // From skills ---------------
    EffectCondition,
    EffectHex,
    EffectWell,
    EffectEnchantment,
    EffectStance,
    EffectShout,
    EffectPreparation,
    EffectSpirit,
    EffectWard,
    // ---------------------------
    EffectEnvironment
};

class Effect
{
private:
    bool UnserializeProp(EffectAttr attr, IO::PropReadStream& stream);
public:
    static void RegisterLua(kaguya::State& state);

    Effect() = default;
    virtual ~Effect() = default;

    /// Gets saved to the DB when player logges out
    bool IsPersistent() const
    {
        return false;
    }

    bool Serialize(IO::PropWriteStream& stream);
    bool Unserialize(IO::PropReadStream& stream);

    uint32_t id_;
    int64_t startTime_;
    int64_t endTime_;
    /// Duration
    uint32_t ticks_;

};

typedef std::forward_list<std::unique_ptr<Effect>> EffectList;

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

class EffectCondition final : public Effect
{

};

class EffectHex final : public Effect
{

};

}

