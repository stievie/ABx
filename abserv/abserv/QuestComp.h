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

#include <memory>
#include <map>
#include <vector>
#include "Quest.h"
#include <sa/Iteration.h>
#include <sa/Noncopyable.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class Player;

namespace Components {

class QuestComp
{
    NON_COPYABLE(QuestComp)
    NON_MOVEABLE(QuestComp)
private:
    Player& owner_;
    std::map<uint32_t, std::unique_ptr<Quest>> activeQuests_;
    // This can be a std::map because we keep this only to check if the player
    // has the requirements for another quest.
    std::map<uint32_t, std::unique_ptr<Quest>> doneQuests_;
    Quest* GetCompletedQuest(uint32_t index) const;
public:
    QuestComp() = delete;
    explicit QuestComp(Player& owner);
    ~QuestComp() = default;

    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message);

    // Player adds a quest
    bool PickupQuest(uint32_t index);
    bool DeleteQuest(uint32_t index);
    bool HaveQuest(uint32_t index) const;
    bool Add(const AB::Entities::Quest& q, AB::Entities::PlayerQuest&& pq);
    bool GetReward(uint32_t questIndex);
    bool SatisfyRequirements(const AB::Entities::Quest& q) const;
    bool SatisfyRequirements(uint32_t index) const;
    bool IsActive(uint32_t index) const;
    bool IsRewarded(uint32_t index) const;
    bool IsRepeatable(uint32_t index) const;
    /// Check if a quest is available for the player
    bool IsAvailable(uint32_t index) const;
    Quest* Get(uint32_t index);
    const Quest* Get(uint32_t index) const;

    template<typename Callback>
    void VisitQuests(Callback&& callback)
    {
        for (const auto& q : doneQuests_)
        {
            if (callback(*q.second) != Iteration::Continue)
                break;
        }
        for (const auto& q : activeQuests_)
        {
            if (callback(*q.second) != Iteration::Continue)
                break;
        }
    }
    template<typename Callback>
    void VisitActiveQuests(Callback&& callback)
    {
        for (const auto& q : activeQuests_)
        {
            if (!q.second->IsActive())
                continue;
            if (callback(*q.second) != Iteration::Continue)
                break;
        }
    }
};

}
}

