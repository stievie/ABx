#include "stdafx.h"
#include "Player.h"

#include "DebugNew.h"

namespace Game {

Player::Player(std::shared_ptr<Net::ProtocolGame> client) :
    client_(std::move(client))
{
}

Player::~Player()
{
}

}
