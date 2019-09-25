#pragma once

#include "Variant.h"
#include <queue>
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
    std::queue<InputItem> queue_;
    std::mutex lock_;
public:
    InputQueue() = default;
    ~InputQueue() = default;

    void Add(InputType type, const Utils::VariantMap& data)
    {
        // Network and Dispatcher Threat
        std::lock_guard<std::mutex> lockClass(lock_);
        queue_.push({ type, data });
    }
    bool Get(InputItem& item)
    {
        // Dispatcher Thread
        if (queue_.empty())
            return false;
        item = queue_.front();
        queue_.pop();
        return true;
    }
};

}

