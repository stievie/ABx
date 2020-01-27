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

#include "InputQueue.h"
#include <sa/Noncopyable.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class Actor;

namespace Components {

class InputComp
{
    NON_COPYABLE(InputComp)
private:
    Actor& owner_;
    InputQueue inputs_;
    void SelectObject(uint32_t sourceId, uint32_t targetId);
    void ClickObject(uint32_t sourceId, uint32_t targetId);
public:
    InputComp() = delete;
    explicit InputComp(Actor& owner) :
        owner_(owner),
        inputs_()
    { }
    ~InputComp() = default;

    void Add(InputType type, Utils::VariantMap&& data)
    {
        inputs_.Add(type, std::move(data));
    }
    void Add(InputType type)
    {
        inputs_.Add(type);
    }
    void Update(uint32_t timeElapsed, Net::NetworkMessage& message);
};

}
}
