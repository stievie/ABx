#pragma once

#include <memory>
#include <AB/ProtocolCodes.h>
#include <AB/Entities/Skill.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class Actor;
class Skill;

namespace Components {

class SkillsComp
{
private:
    Actor& owner_;
    AB::GameProtocol::SkillError lastError_;
    /// Index in users skill bar, 0 based
    int lastSkillIndex_;
    bool startDirty_;
    bool endDirty_;
    bool usingSkill_;
    int32_t newRecharge_;
    int64_t lastSkillTime_;
    std::weak_ptr<Skill> lastSkill_;
    void OnIncMorale(int morale);
public:
    SkillsComp() = delete;
    explicit SkillsComp(Actor& owner);
    // non-copyable
    SkillsComp(const SkillsComp&) = delete;
    SkillsComp& operator=(const SkillsComp&) = delete;
    ~SkillsComp() = default;

    void Update(uint32_t timeElapsed);
    AB::GameProtocol::SkillError UseSkill(int index, bool ping);
    void Cancel();
    void CancelWhenChangingState();
    bool IsUsing();
    void Write(Net::NetworkMessage& message);
    int64_t GetLastSkillTime() const { return lastSkillTime_; }
    bool Interrupt(AB::Entities::SkillType type);
    /// Return currently using skill. Maybe nullptr.
    Skill* GetCurrentSkill();
    void GetResources(int& maxHealth, int& maxEnergy);
    void GetSkillRecharge(Skill* skill, int32_t& recharge);
    void GetSkillCost(Skill* skill,
        int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp);
};

}
}
