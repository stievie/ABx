#pragma once

namespace Game {

class Actor;
class Skill;

namespace Components {

class HealComp
{
private:
    struct HealItem
    {
        uint32_t actorId;
        // Skill or Effect index
        uint32_t index;
        int value;
        int64_t tick;
    };
    Actor& owner_;
    std::vector<HealItem> healings_;
public:
    HealComp() = delete;
    explicit HealComp(Actor& owner) :
        owner_(owner)
    { }
    // non-copyable
    HealComp(const HealComp&) = delete;
    HealComp& operator=(const HealComp&) = delete;
    ~HealComp() = default;

    void Healing(Actor* source, uint32_t index, int value);
    void Update(uint32_t /* timeElapsed */) { }
    void Write(Net::NetworkMessage& message);
};

}
}

