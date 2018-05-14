#pragma once

#include "Variant.h"
#include <queue>

namespace Game {

enum InputType
{
    InputTypeNone = 0,
    InputTypeMove = 1,
    InputTypeTurn = 2,
    InputTypeUseSkill = 3,
    InputTypeCancelSkill = 4,
    InputTypeAttack = 5,
    InputTypeCancelAttack = 6,
    InputTypeSelect = 7,
    InputTypeDirection = 8,
    InputTypeCommand = 9,
    InputTypeGoto = 10,
    InputTypeFollow = 11
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
        // Network Threat
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

