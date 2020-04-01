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
#include <memory>
#include <sa/Noncopyable.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class GameObject;
class Actor;

namespace Components {

class SelectionComp
{
    NON_COPYABLE(SelectionComp)
private:
    Actor& owner_;
    uint32_t prevObjectId_{ 0 };
    uint32_t currObjectId_{ 0 };
public:
    SelectionComp() = delete;
    explicit SelectionComp(Actor& owner) :
        owner_(owner)
    { }
    ~SelectionComp();

    bool SelectObject(uint32_t targetId);
    bool ClickObject(uint32_t targetId);

    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message);
    uint32_t GetSelectedObjectId() const { return currObjectId_; }
    GameObject* GetSelectedObject() const;
};

}
}
