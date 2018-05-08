#pragma once

#include "GameObject.h"
#include "Effect.h"
#include "SkillBar.h"
#include "InputQueue.h"
#include <AB/ProtocolCodes.h>

namespace Game {

static const int MOVE_ANGLES[] = { 0, 45, 90, 135, 180, 225, 270, 315 };
static const float BASE_SPEED = 150.0f;

/// Player, NPC, Monster some such
class Creature : public GameObject
{
private:
    void DeleteEffect(uint32_t index);
    void DoCollisions();
    uint8_t moveDir_;
    uint8_t turnDir_;
protected:
    std::vector<Math::Vector3> wayPoints_;
    virtual void HandleCommand(AB::GameProtocol::CommandTypes type,
        const std::string& command, Net::NetworkMessage& message) {
        AB_UNUSED(type);
        AB_UNUSED(command);
        AB_UNUSED(message);
    }
public:
    static void RegisterLua(kaguya::State& state);

    Creature();

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

    Skill* GetSkill(uint32_t index)
    {
        if (index < PLAYER_MAX_SKILLS)
            return skills_[index];
        return nullptr;
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
