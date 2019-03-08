#include "stdafx.h"
#include "TriggerComp.h"
#include "Npc.h"

namespace Game {
namespace Components {

void TriggerComp::DoTrigger(Actor * other)
{
    if (!other)
        return;

    int64_t tick = Utils::Tick();
    int64_t lasTrigger = triggered_[other->id_];
    if (static_cast<uint32_t>(tick - lasTrigger) > retriggerTimeout_)
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
