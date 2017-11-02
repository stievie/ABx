#pragma once

#include "ProtocolGame.h"
#include <memory>
#include "GameObject.h"

namespace Game {

class Player final : public GameObject
{
private:
    std::shared_ptr<Net::ProtocolGame> client_;
public:
    explicit Player(std::shared_ptr<Net::ProtocolGame> client);
    ~Player();
    // non-copyable
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;


    uint32_t accountId_;
};

}
