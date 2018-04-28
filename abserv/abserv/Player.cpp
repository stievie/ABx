#include "stdafx.h"
#include "Player.h"
#include "Logger.h"
#include "Chat.h"
#include "Random.h"
#include "MailBox.h"
#include "PlayerManager.h"

#include "DebugNew.h"

namespace Game {

Player::Player(std::shared_ptr<Net::ProtocolGame> client) :
    Creature(),
    client_(std::move(client)),
    lastPing_(0),
    mailBox_(nullptr)
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
    switch (type)
    {
    case AB::GameProtocol::CommandTypeChatGeneral:
    {
        std::shared_ptr<ChatChannel> channel = Chat::Instance.Get(ChannelMap, GetGame()->id_);
        if (channel)
        {
            channel->Talk(this, command);
        }
        break;
    }
    case AB::GameProtocol::CommandTypeRoll:
    {
        if (Utils::IsNumber(command))
        {
            int max = std::stoi(command);
            if (max >= ROLL_MIN && max <= ROLL_MAX)
            {
                int res = static_cast<int>(Utils::Random::Instance.GetFloat() * (float)max) + 1;
                message.AddByte(AB::GameProtocol::ServerMessage);
                message.AddByte(AB::GameProtocol::ServerMessageTypeRoll);
                message.AddString(GetName());
                message.AddString(std::to_string(res) + ":" + std::to_string(max));
            }
        }
        break;
    }
    case AB::GameProtocol::CommandTypeAge:
    {
        uint32_t playTime = static_cast<uint32_t>(data_.onlineTime) + static_cast<uint32_t>((Utils::AbTick() - loginTime_) / 1000);
        uint32_t age = static_cast<uint32_t>(Utils::AbTick() - data_.creation);

        Net::NetworkMessage nmsg;
        nmsg.AddByte(AB::GameProtocol::ServerMessage);
        nmsg.AddByte(AB::GameProtocol::ServerMessageTypeAge);
        nmsg.AddString(GetName());
        nmsg.AddString(std::to_string(age) + ":" + std::to_string(playTime));
        client_->WriteToOutput(nmsg);
        break;
    }
    case AB::GameProtocol::CommandTypeChatWhisper:
    {
        size_t p = command.find(',');
        if (p != std::string::npos)
        {
            std::string name = command.substr(0, p);
            std::string msg = Utils::LeftTrim(command.substr(p + 1, std::string::npos));
            std::shared_ptr<Player> target = PlayerManager::Instance.GetPlayerByName(name);
            if (target)
            {
                std::shared_ptr<ChatChannel> channel = Chat::Instance.Get(ChannelWhisper, target->id_);
                if (channel)
                {
                    if (channel->Talk(this, msg))
                    {
                        Net::NetworkMessage nmsg;
                        nmsg.AddByte(AB::GameProtocol::ServerMessage);
                        nmsg.AddByte(AB::GameProtocol::ServerMessageTypePlayerGotMessage);
                        nmsg.AddString(name);
                        nmsg.AddString(msg);
                        client_->WriteToOutput(nmsg);
                    }
                }
            }
            else
            {
                Net::NetworkMessage nmsg;
                nmsg.AddByte(AB::GameProtocol::ServerMessage);
                nmsg.AddByte(AB::GameProtocol::ServerMessageTypePlayerNotOnline);
                nmsg.AddString(GetName());
                nmsg.AddString(name);
                client_->WriteToOutput(nmsg);
            }
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
