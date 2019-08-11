#pragma once

#include <stdint.h>
#include <map>
#include "Iteration.h"

namespace Game {

class GameObject;

namespace Components {

/// NPCs can be used a trigger box. This component calls Actor::OnTrigger() when it collides with the collision shape.
class TriggerComp
{
private:
    GameObject& owner_;
    std::map<uint32_t, int64_t> triggered_;
    uint32_t lastCheck_{ 0 };
    void DoTrigger(GameObject* other);
public:
    TriggerComp() = delete;
    explicit TriggerComp(GameObject& owner) :
        owner_(owner)
    { }
    // non-copyable
    TriggerComp(const TriggerComp&) = delete;
    TriggerComp& operator=(const TriggerComp&) = delete;
    ~TriggerComp() = default;

    void Update(uint32_t timeElapsed);
    /// Get all object IDs inside the collision shape
    template <typename Callback>
    void VisitObjectInside(const Callback& callback)
    {
        for (const auto& i : triggered_)
        {
            if (callback(i.first) != Iteration::Continue)
                break;
        }
    }

    void OnCollide(GameObject* other);

    bool trigger_{ false };
    /// Time in ms the same Actor can retrigger
    uint32_t retriggerTimeout_{ std::numeric_limits<uint32_t>::max() };   // By default never retrigger
};

}
}

