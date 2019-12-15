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

    bool Add(const AB::Entities::Quest& q, AB::Entities::PlayerQuest&& pq);
    bool GetReward(uint32_t questIndex);

    template<typename Callback>
    void VisitQuests(const Callback& callback)
    {
        for (const auto& q : quests_)
        {
            if (callback(*q.second) != Iteration::Continue)
                break;
        }
    }
};

}
}

