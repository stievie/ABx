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
        owner_.OnTrigger(other);
        triggered_[other->id_] = tick;
    }
}

void TriggerComp::Update(uint32_t timeElapsed)
{
    if (triggered_.size() == 0)
        return;

    lastCheck_ += timeElapsed;
    // Check all 200ms
    if (lastCheck_ < 200)
        return;

    // Check if objects are still inside
    auto game = owner_.GetGame();
    Math::Vector3 move;
    for (auto it = triggered_.begin(); it != triggered_.end(); )
    {
        auto o = game->GetObjectById((*it).first);
        // If not inside remove from triggered list
        if (!o || !owner_.Collides(o.get(), Math::Vector3::Zero, move))
            triggered_.erase(it++);
        else
            ++it;
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
