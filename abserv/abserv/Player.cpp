#include "stdafx.h"
#include "Player.h"
#include "Logger.h"

#include "DebugNew.h"

namespace Game {

Player::Player(std::shared_ptr<Net::ProtocolGame> client) :
    Creature(),
    client_(std::move(client))
{
}

Player::~Player()
{
#ifdef DEBUG_GAME
//    LOG_DEBUG << std::endl;
#endif
}

void Player::Logout()
{
    if (auto g = GetGame())
        g->PlayerLeave(id_);
    client_->Logout();
}

void Player::RegisterLua(kaguya::State& state)
{
    state["Player"].setClass(kaguya::UserdataMetatable<Player, Creature>()
    );
}

}
