#pragma once

#include "Damage.h"

namespace Game {

class Actor;

namespace Components {

class DamageComp
{
private:
    struct DamageItem
    {
        DamageType type;
        int value;
        // The skill causing this damage. If 0 it's a melee damage.
        uint32_t skillIndex;
        int64_t tick;
    };
    Actor& owner_;
    std::vector<DamageItem> damages_;
public:
    DamageComp() = delete;
    explicit DamageComp(Actor& owner) :
        owner_(owner),
        lastDamage_(0)
    { }
    // non-copyable
    DamageComp(const DamageComp&) = delete;
    DamageComp& operator=(const DamageComp&) = delete;
    ~DamageComp() = default;

    void Update(uint32_t /* timeElapsed */) { }
    void ApplyDamage(DamageType type, int value, uint32_t skillIndex = 0);
    void Touch();
    /// How long the actor didn't get damage in ms
    uint32_t NoDamageTime() const;
    void Write(Net::NetworkMessage& message);

    int64_t lastDamage_;
};

}
}

