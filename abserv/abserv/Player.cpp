#include "stdafx.h"
#include "Player.h"
#include "Logger.h"
#include "Chat.h"

#include "DebugNew.h"

namespace Game {

Player::Player(std::shared_ptr<Net::ProtocolGame> client) :
    Creature(),
    client_(std::move(client)),
    lastPing_(0)
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

void Player::Ping()
{
    lastPing_ = Utils::AbTick();
    Net::NetworkMessage msg;
    msg.AddByte(AB::GameProtocol::GamePong);
    client_->WriteToOutput(msg);
}

void Player::HandleCommand(AB::GameProtocol::CommandTypes type,
    const std::string& command, Net::NetworkMessage& message)
{
    AB_UNUSED(message);
    switch (type)
    {
    case AB::GameProtocol::CommandTypeChatGeneral:
    {
        ChatChannel* channel = Chat::Instance.Get(ChannelMap, GetGame()->id_);
        if (channel)
        {
            channel->Talk(this, command);
        }
        break;
    }
    }
}

void Player::RegisterLua(kaguya::State& state)
{
    state["Player"].setClass(kaguya::UserdataMetatable<Player, Creature>()
    );
}

}
