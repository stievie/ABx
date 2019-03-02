#pragma once

#include "PropStream.h"
#include <forward_list>
#include <AB/Entities/Effect.h>
#include "Script.h"

namespace Game {

class Actor;
class Skill;

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
    kaguya::State luaState_;
    std::shared_ptr<Script> script_;
    std::weak_ptr<Actor> target_;
    std::weak_ptr<Actor> source_;
    bool persistent_;
    bool haveUpdate_;
    bool haveSkillCost_;
    bool UnserializeProp(EffectAttr attr, IO::PropReadStream& stream);
    void InitializeLua();
public:
    static void RegisterLua(kaguya::State& state);

    Effect() = delete;
    explicit Effect(const AB::Entities::Effect& effect) :
        persistent_(false),
        haveUpdate_(false),
        haveSkillCost_(false),
        data_(effect),
        startTime_(0),
        endTime_(0),
        ticks_(0),
        ended_(false),
        cancelled_(false)
    {
        InitializeLua();
    }
    ~Effect() = default;

    /// Gets saved to the DB when player logs out
    bool IsPersistent() const
    {
        return persistent_;
    }

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
    void GetSkillCost(Skill* skill, int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp);

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

