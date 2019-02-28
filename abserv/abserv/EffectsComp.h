#pragma once

#include "Effect.h"

namespace Game {

class Actor;
class Skill;

namespace Components {

class EffectsComp
{
private:
    static constexpr AB::Entities::EffectCategory SINGLEEFFECT_START = AB::Entities::EffectPreparation;
    static constexpr AB::Entities::EffectCategory SINGLEEFFECT_END = AB::Entities::EffectWeaponSpell;
    Actor& owner_;
    EffectList addedEffects_;
    EffectList removedEffects_;
    void RemoveAllOfCategory(AB::Entities::EffectCategory categroy);
public:
    explicit EffectsComp(Actor& owner) :
        owner_(owner)
    { }
    ~EffectsComp() = default;

    void AddEffect(std::shared_ptr<Actor> source, uint32_t index);
    void DeleteEffect(uint32_t index);
    /// Remove effect before it ended
    void RemoveEffect(uint32_t index);
    std::shared_ptr<Effect> GetLast(AB::Entities::EffectCategory category);
    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message);
    void GetSkillCost(Skill* skill, int16_t& energy, int16_t& adrenaline, int16_t& activation, int16_t& overcast);

    EffectList effects_;
};

}
}
