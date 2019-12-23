#pragma once

#include <memory>
#include <map>
#include "Quest.h"
#include <sa/Iteration.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class Player;

namespace Components {

class QuestComp
{
private:
    Player& owner_;
    std::map<uint32_t, std::unique_ptr<Quest>> quests_;
public:
    QuestComp() = delete;
    explicit QuestComp(Player& owner);
    // non-copyable
    QuestComp(const QuestComp&) = delete;
    QuestComp& operator=(const QuestComp&) = delete;
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
    void VisitQuests(const Callback& callback)
    {
        for (const auto& q : quests_)
        {
            if (callback(*q.second) != Iteration::Continue)
                break;
        }
    }
    template<typename Callback>
    void VisitActiveQuests(const Callback& callback)
    {
        for (const auto& q : quests_)
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

