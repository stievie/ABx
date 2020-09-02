/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "TriggerComp.h"
#include "GameObject.h"
#include "Game.h"
#include <sa/time.h>

namespace Game {
namespace Components {

TriggerComp::TriggerComp(GameObject& owner) :
    owner_(owner)
{
    owner_.SubscribeEvent<void(GameObject*)>(EVENT_ON_COLLIDE, std::bind(&TriggerComp::OnCollide, this, std::placeholders::_1));
}

void TriggerComp::DoTrigger(GameObject* other)
{
    if (!other)
        return;

    const int64_t tick = sa::time::tick();
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
            auto* o = game->GetObject<GameObject>((*it).first);
            // If not inside remove from triggered list
            if (!o)
            {
                // This object is gone
                triggered_.erase(it++);
            }
            else if (!owner_.Collides(o, Math::Vector3::Zero, move))
            {
                // No longer collides
                triggered_.erase(it++);
                owner_.CallEvent<void(GameObject*)>(EVENT_ON_LEFTAREA, o);
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
