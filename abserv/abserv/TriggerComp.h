#pragma once

namespace Game {

class Actor;

namespace Components {

/// NPCs can be used a trigger box. This component calls Actor::OnTrigger() when it collides with the collision shape.
class TriggerComp
{
private:
    Actor& owner_;
    std::map<uint32_t, int64_t> triggered_;
    void DoTrigger(Actor* other);
public:
    TriggerComp() = delete;
    explicit TriggerComp(Actor& owner) :
        owner_(owner),
        retriggerTimeout_(1000)
    { }
    // non-copyable
    TriggerComp(const TriggerComp&) = delete;
    TriggerComp& operator=(const TriggerComp&) = delete;
    ~TriggerComp() = default;

    void OnCollide(Actor* other);

    bool trigger_;
    /// Time in ms the same Actor can retrigger
    uint32_t retriggerTimeout_;
};

}
}

