#pragma once

#include <AB/ProtocolCodes.h>

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
    int16_t newRecharge_;
    std::weak_ptr<Skill> lastSkill_;
public:
    explicit SkillsComp(Actor& owner) :
        owner_(owner),
        lastError_(AB::GameProtocol::SkillErrorNone),
        lastSkillIndex_(-1),
        startDirty_(false),
        endDirty_(false),
        usingSkill_(false),
        newRecharge_(0)
    { }
    ~SkillsComp() = default;
    void Update(uint32_t timeElapsed);
    AB::GameProtocol::SkillError UseSkill(int index);
    void Write(Net::NetworkMessage& message);
};

}
}
