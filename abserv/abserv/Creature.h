#pragma once

#include "GameObject.h"
#include "Effect.h"
#include "SkillBar.h"
#include "InputQueue.h"
#include <AB/ProtocolCodes.h>

namespace Game {

/// Player, NPC, Monster some such
class Creature : public GameObject
{
private:
    static constexpr float BaseSpeed = 150.0f;
    void DeleteEffect(uint32_t index);
    void DoCollisions();
    uint8_t moveDir_;
    uint8_t turnDir_;
protected:
    std::vector<Math::Vector3> wayPoints_;
    virtual void HandleCommand(AB::GameProtocol::CommandTypes type,
        const std::string& command, Net::NetworkMessage& message,
        AB::GameProtocol::CreatureState& newState) {
        AB_UNUSED(type);
        AB_UNUSED(command);
        AB_UNUSED(message);
        AB_UNUSED(newState);
    }
public:
    static void RegisterLua(kaguya::State& state);

    Creature();

    void SetGame(std::shared_ptr<Game> game) override
    {
        GameObject::SetGame(game);
    }

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

    float GetActualMoveSpeed() const
    {
        // TODO:
        return speed_;
    }

    virtual uint32_t GetLevel() const { return 0; }

    uint32_t GetProfIndex() const final override
    {
        return skills_.prof1_.index;
    }
    uint32_t GetProf2Index() const final override
    {
        return skills_.prof2_.index;
    }
    Skill* GetSkill(uint32_t index)
    {
        if (index < PLAYER_MAX_SKILLS)
            return skills_[index];
        return nullptr;
    }
    SkillBar* GetSkillBar()
    {
        return &skills_;
    }
    void AddEffect(std::shared_ptr<Creature> source, uint32_t index, uint32_t baseDuration);
    /// Remove effect before it ended
    void RemoveEffect(uint32_t index);

    /// Move in direction of rotation
    bool Move(float speed, const Math::Vector3& amount);
    void Turn(float angle);
    void SetDirection(float worldAngle);

    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;

    InputQueue inputs_;
    AB::GameProtocol::CreatureState creatureState_;
    int64_t lastStateChange_;
    std::weak_ptr<GameObject> selectedObject_;
    std::weak_ptr<GameObject> followedObject_;

    EffectList effects_;
    SkillBar skills_;
    uint32_t energy_;
    uint32_t health_;
    uint32_t adrenaline_;
    uint32_t overcast_;

    float speed_ = 1.0f;
    /// Effects may influence the cast spells speed
    float castSpeedFactor_ = 1.0f;
    /// For any skill
    float skillSpeedFactor_ = 1.0f;
    /// Effects may influence the attack speed
    float attackSpeedFactor_ = 1.0f;

    bool Serialize(IO::PropWriteStream& stream) override;
};

}
