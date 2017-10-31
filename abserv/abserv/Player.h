#pragma once

#include "ProtocolGame.h"
#include <memory>
#include "GameObject.h"

namespace Game {

class Player : public GameObject
{
private:
    std::shared_ptr<Net::ProtocolGame> client_;
public:
    explicit Player(std::shared_ptr<Net::ProtocolGame> client);
    ~Player();
};

}
