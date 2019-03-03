#pragma once

#include "Damage.h"

namespace Game {

class Actor;

namespace Components {

class DamageComp
{
private:
    Actor& owner_;
public:
    DamageComp() = delete;
    explicit DamageComp(Actor& owner) :
        owner_(owner),
        lastDamage_(0)
    { }
    ~DamageComp() = default;

    void Update(uint32_t /* timeElapsed */) { }
    void ApplyDamage(DamageType type, int value);
    void Touch();
    /// How long the actor didn't get damage in ms
    uint32_t NoDamageTime() const;

    int64_t lastDamage_;
};

}
}

