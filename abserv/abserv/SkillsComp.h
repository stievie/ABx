/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <memory>
#include <AB/ProtocolCodes.h>
#include <AB/Entities/Skill.h>
#include <sa/Noncopyable.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class Actor;
class Skill;

namespace Components {

class SkillsComp
{
    NON_COPYABLE(SkillsComp)
private:
    Actor& owner_;
    AB::GameProtocol::SkillError lastError_;
    /// Index in users skill bar, 0 based
    int lastSkillIndex_;
    bool startDirty_;
    bool endDirty_;
    bool usingSkill_;
    uint32_t newRecharge_;
    int64_t lastSkillTime_;
    std::weak_ptr<Skill> lastSkill_;
    void OnIncMorale(int morale);
public:
    SkillsComp() = delete;
    explicit SkillsComp(Actor& owner);
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
    void GetSkillRecharge(Skill* skill, uint32_t& recharge);
    void GetSkillCost(Skill* skill,
        int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp);
};

}
}
