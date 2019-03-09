#include "stdafx.h"
#include "TriggerComp.h"
#include "Actor.h"

namespace Game {
namespace Components {

void TriggerComp::DoTrigger(Actor* other)
{
    if (!other)
        return;

    const int64_t tick = Utils::Tick();
    const int64_t lastTrigger = triggered_[other->id_];
    if (static_cast<uint32_t>(tick - lastTrigger) > retriggerTimeout_)
        owner_.OnTrigger(other);

    triggered_[other->id_] = tick;

    // Delete old
    for (auto it = triggered_.begin(); it != triggered_.end(); )
    {
        if (tick - (*it).second > 10000)
            triggered_.erase(it++);
        else
            ++it;
    }
}

void TriggerComp::OnCollide(Actor* other)
{
    if (trigger_ && other)
        DoTrigger(other);
}

}
}
