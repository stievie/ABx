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
    NON_MOVEABLE(TriggerComp)
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
    void VisitObjectInside(Callback&& callback)
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

