#pragma once

#include "PropStream.h"
#include <forward_list>
#include <vector>
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

enum EffectCategory : uint8_t
{
    EffectNone = 0,
    // From skills ---------------
    EffectCondition = 1,
    EffectEnchantment = 2,
    EffectHex = 3,
    EffectPreparation = 4,
    EffectShout = 5,
    EffectSpirit = 6,
    EffectStance = 7,
    EffectWard = 8,
    EffectWell = 9,
    // ---------------------------
    EffectEnvironment = 254
};

class Effect
{
private:
    kaguya::State luaState_;
    Creature* target_;
    bool UnserializeProp(EffectAttr attr, IO::PropReadStream& stream);
    void InitializeLua();
public:
    static void RegisterLua(kaguya::State& state);

    Effect(uint32_t id) :
        id_(id),
        ended_(false),
        cancelled_(false)
    {}
    ~Effect() = default;

    /// Gets saved to the DB when player logs out
    bool IsPersistent() const
    {
        return false;
    }

    bool LoadScript(const std::string& fileName);
    void Update(uint32_t timeElapsed);
    void Start(Creature* target, uint32_t ticks);
    void Remove();

    bool Serialize(IO::PropWriteStream& stream);
    bool Unserialize(IO::PropReadStream& stream);

    uint32_t id_;
    int64_t startTime_;
    int64_t endTime_;
    /// Duration
    uint32_t ticks_;
    bool ended_;
    bool cancelled_;

    EffectCategory category_;

    int64_t GetStartTime() const { return startTime_; }
    int64_t GetEndTime() const { return endTime_; }
    uint32_t GetTicks() const { return ticks_; }
};

/// effects are fist-in-last-out
typedef std::vector<std::unique_ptr<Effect>> EffectList;

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

