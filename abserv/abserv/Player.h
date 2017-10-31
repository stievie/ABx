#pragma once

#include "ProtocolGame.h"
#include "Game.h"
#include <memory>
#include "Vector3.h"
#include "Quaternion.h"

namespace Game {

class Player
{
private:
    std::shared_ptr<Net::ProtocolGame> client_;
    std::shared_ptr<Game> game_;
    Math::Vector3 position_;
    Math::Quaternion rotation_;
public:
    explicit Player(std::shared_ptr<Net::ProtocolGame> client);
    ~Player();
};

}
