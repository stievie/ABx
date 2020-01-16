#pragma once

#include <stdint.h>
#include <map>
#include <sa/Iteration.h>
#include <sa/Noncopyable.h>

namespace Game {

class GameObject;

namespace Components {

/// NPCs can be used a trigger box. This component calls Actor::OnTrigger() when it collides with the collision shape.
class TriggerComp
{
    NON_COPYABLE(TriggerComp)
private:
    GameObject& owner_;
    std::map<uint32_t, int64_t> triggered_;
    uint32_t lastCheck_{ 0 };
    void DoTrigger(GameObject* other);
    void OnCollide(GameObject* other);
public:
    TriggerComp() = delete;
    explicit TriggerComp(GameObject& owner);
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

    bool trigger_{ false };
    /// Time in ms the same Actor can retrigger
    uint32_t retriggerTimeout_{ std::numeric_limits<uint32_t>::max() };   // By default never retrigger
};

}
}

