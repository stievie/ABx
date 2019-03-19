#pragma once

#include <AB/ProtocolCodes.h>
#include <AB/Entities/Skill.h>

namespace Game {

class Actor;
class Skill;

namespace Components {

class SkillsComp
{
private:
    Actor& owner_;
    AB::GameProtocol::SkillError lastError_;
    int lastSkillIndex_;
    bool startDirty_;
    bool endDirty_;
    bool usingSkill_;
    int32_t newRecharge_;
    int64_t lastSkillTime_;
    std::weak_ptr<Skill> lastSkill_;
public:
    SkillsComp() = delete;
    explicit SkillsComp(Actor& owner) :
        owner_(owner),
        lastError_(AB::GameProtocol::SkillErrorNone),
        lastSkillIndex_(-1),
        startDirty_(false),
        endDirty_(false),
        usingSkill_(false),
        newRecharge_(0),
        lastSkillTime_(0)
    { }
    // non-copyable
    SkillsComp(const SkillsComp&) = delete;
    SkillsComp& operator=(const SkillsComp&) = delete;
    ~SkillsComp() = default;

    void Update(uint32_t timeElapsed);
    AB::GameProtocol::SkillError UseSkill(int index);
    void Cancel();
    void CancelWhenChangingState();
    bool IsUsing();
    void Write(Net::NetworkMessage& message);
    int64_t GetLastSkillTime() const { return lastSkillTime_; }
    bool Interrupt(AB::Entities::SkillType type);
    /// Return currently using skill. Maybe nullptr.
    Skill* GetCurrentSkill();
};

}
}
