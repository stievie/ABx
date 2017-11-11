#pragma once

#include "GameObject.h"
#include "Effect.h"
#include "SkillBar.h"

namespace Game {

/// Player, NPC, Monster some such
class Creature : public GameObject
{
public:
    static void RegisterLua(kaguya::State& state);

    /// Move speed: 1 = normal speed
    float GetSpeed() const { return speed_; }
    void SetSpeed(float value) { speed_ = value; }
    uint32_t GetEnergy() const { return energy_; }
    void SetEnergy(uint32_t value) { energy_ = value; }
    uint32_t GetHealth() const { return health_; }
    void SetHealth(uint32_t value) { health_ = value; }
    uint32_t GetAdrenaline() const { return adrenaline_; }
    void SetAdrenaline(uint32_t value) { adrenaline_ = value; }
    uint32_t GetOvercast() const { return overcast_; }
    void SetOvercast(uint32_t value) { overcast_ = value; }

    Skill* GetSkill(uint32_t index)
    {
        if (index >= 0 && index < PLAYER_MAX_SKILLS)
            return skills_[index];
        return nullptr;
    }

    EffectList effects_;
    SkillBar skills_;
    float speed_ = 1.0f;
    uint32_t energy_;
    uint32_t health_;
    uint32_t adrenaline_;
    uint32_t overcast_;
};

}
