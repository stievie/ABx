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

#include <abscommon/Variant.h>
#include <eastl.hpp>
#include <EASTL/queue.h>
#include <mutex>

namespace Game {

enum class InputType
{
    None = 0,
    Move,
    Turn,
    UseSkill,
    Attack,
    Cancel,        // Cancel skill/attack/follow
    Select,
    ClickObject,
    Direction,
    Command,
    Goto,
    Follow,
    SetState,
    Interact,
};

enum InputDataType
{
    InputDataDirection   = 1,
    InputDataSkillIndex  = 3,    // The index of the skill in the skill bar
    InputDataObjectId    = 4,
    InputDataObjectId2   = 5,
    InputDataCommandType = 6,
    InputDataCommandData = 7,
    InputDataVertexX     = 8,
    InputDataVertexY     = 9,
    InputDataVertexZ     = 10,
    InputDataState       = 11,
    InputDataPingTarget  = 12,
};

struct InputItem
{
    InputType type;
    Utils::VariantMap data;
};

class InputQueue
{
private:
    ea::queue<InputItem> queue_;
    std::mutex lock_;
public:
    InputQueue();
    ~InputQueue();

    void Add(InputType type, Utils::VariantMap&& data)
    {
        // Network and Dispatcher Threat
        std::scoped_lock lock(lock_);
        queue_.push({ type, std::move(data) });
    }
    void Add(InputType type)
    {
        // Network and Dispatcher Threat
        std::scoped_lock lock(lock_);
        queue_.push({ type, Utils::VariantMapEmpty });
    }
    bool Get(InputItem& item);
};

}

