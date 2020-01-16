#pragma once

#include <stdint.h>
#include <vector>
#include <sa/Noncopyable.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class Actor;
class Skill;

namespace Components {

class HealComp
{
    NON_COPYABLE(HealComp)
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
    ~HealComp() = default;

    void Healing(Actor* source, uint32_t index, int value);
    void Update(uint32_t /* timeElapsed */) { }
    void Write(Net::NetworkMessage& message);
};

}
}

