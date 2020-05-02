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

#include <vector>
#include <stdint.h>
#include <sa/Noncopyable.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class Actor;

namespace Components {

class ProgressComp
{
    NON_COPYABLE(ProgressComp)
    NON_MOVEABLE(ProgressComp)
private:
    enum class ProgressType
    {
        XPIncrease,
        GotSkillPoint,
        LevelAdvance,
        TitleAdvance,
        AttributePointGain,
    };
    struct ProgressItem
    {
        ProgressType type;
        int value;
    };
    std::vector<ProgressItem> items_;
    Actor& owner_;
    unsigned deaths_{ 0 };
private:
    /// The owner died
    void OnDied(Actor*, Actor*);
    /// A foe was killed nearby
    void OnKilledFoe(Actor* foe, Actor* killer);
public:
    ProgressComp() = delete;
    explicit ProgressComp(Actor& owner);
    ~ProgressComp() = default;

    void Update(uint32_t /* timeElapsed */) { }
    void Write(Net::NetworkMessage& message);

    void AddXp(int value);
    /// Somewhere close an enemy died, so we get some XP
    void AddXpForKill(Actor* victim);
    void AddSkillPoint();
    void AdvanceLevel();
    unsigned GetDeaths() const { return deaths_; }
};

}
}
