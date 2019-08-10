#pragma once

#include <memory>
#include <vector>
#include "Quest.h"

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
    std::vector<std::unique_ptr<Quest>> quests_;
public:
    QuestComp() = delete;
    explicit QuestComp(Player& owner) :
        owner_(owner)
    { }
    // non-copyable
    QuestComp(const QuestComp&) = delete;
    QuestComp& operator=(const QuestComp&) = delete;
    ~QuestComp() = default;

    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message);
};

}
}

