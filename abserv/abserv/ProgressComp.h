#pragma once

#include <vector>
#include <stdint.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class Actor;

namespace Components {

class ProgressComp
{
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
    void OnDied();
    /// A foe was killed nearby
    void OnKilledFoe(Actor* foe, Actor* killer);
public:
    ProgressComp() = delete;
    explicit ProgressComp(Actor& owner);
    // non-copyable
    ProgressComp(const ProgressComp&) = delete;
    ProgressComp& operator=(const ProgressComp&) = delete;
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
