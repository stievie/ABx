#include "stdafx.h"
#include "TriggerComp.h"
#include "GameObject.h"
#include "Game.h"

namespace Game {
namespace Components {

void TriggerComp::DoTrigger(GameObject* other)
{
    if (!other)
        return;

    const int64_t tick = Utils::Tick();
    const auto it = triggered_.find(other->id_);
    const int64_t lastTrigger = (it != triggered_.end()) ? (*it).second : 0;
    if (lastTrigger == 0 || static_cast<uint32_t>(tick - lastTrigger) > retriggerTimeout_)
    {
        owner_.CallEvent<void(GameObject*)>(EVENT_ON_TRIGGER, other);
        triggered_[other->id_] = tick;
    }
}

void TriggerComp::Update(uint32_t timeElapsed)
{
    if (triggered_.size() == 0)
        return;

    lastCheck_ += timeElapsed;
    // Keep objects at least 2 ticks inside
    if (lastCheck_ < NETWORK_TICK * 2)
        return;

    // Check if objects are still inside
    auto game = owner_.GetGame();
    if (!game)
    {
        // Game over
        triggered_.clear();
    }
    else
    {
        Math::Vector3 move;
        for (auto it = triggered_.begin(); it != triggered_.end(); )
        {
            auto o = game->GetObjectById((*it).first);
            // If not inside remove from triggered list
            if (!o)
            {
                // This object is gone
                triggered_.erase(it++);
            }
            else if (!owner_.Collides(o.get(), Math::Vector3::Zero, move))
            {
                // No longer collides
                triggered_.erase(it++);
                owner_.CallEvent<void(GameObject*)>(EVENT_ON_LEFTAREA, o.get());
            }
            else
                ++it;
        }
    }
    lastCheck_ = 0;
}

void TriggerComp::OnCollide(GameObject* other)
{
    if (trigger_ && other)
        DoTrigger(other);
}

}
}
