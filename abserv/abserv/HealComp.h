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
#include <vector>
#include <sa/Noncopyable.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class Actor;
class Skill;

namespace Components {

class HealComp
{
    NON_COPYABLE(HealComp)
    NON_MOVEABLE(HealComp)
private:
    struct HealItem
    {
        uint32_t actorId;
        // Skill or Effect index
        uint32_t index;
        int value;
        int64_t tick;
    };
    Actor& owner_;
    std::vector<HealItem> healings_;
public:
    HealComp() = delete;
    explicit HealComp(Actor& owner) :
        owner_(owner)
    { }
    ~HealComp() = default;

    void Healing(Actor* source, uint32_t index, int value);
    void Update(uint32_t /* timeElapsed */) { }
    void Write(Net::NetworkMessage& message);
};

}
}

