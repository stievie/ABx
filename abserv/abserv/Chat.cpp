#include "stdafx.h"
#include "Chat.h"
#include "Game.h"
#include "Player.h"
#include "GameManager.h"
#include "PlayerManager.h"

namespace Game {

Chat Chat::Instance;

std::shared_ptr<ChatChannel> Chat::Get(uint8_t type, uint32_t id)
{
    if (type == ChannelWhisper)
    {
        return std::make_shared<WhisperChatChannel>(id);
    }

    uint64_t channelId = ((uint64_t)type << 32) | id;
    auto it = channels_.find(channelId);
    if (it != channels_.end())
        return (*it).second;

    std::shared_ptr<ChatChannel> c;
    if (type == ChannelMap || type == ChannelTrade)
        c = std::make_shared<GameChatChannel>(id);
    else
        c = std::make_shared<ChatChannel>(id);
    channels_.emplace(channelId, c);
    return c;
}

void Chat::Remove(uint8_t type, uint32_t id)
{
    uint64_t channelId = ((uint64_t)type << 32) | id;
    auto it = channels_.find(channelId);
    if (it != channels_.end())
        channels_.erase(it);
}

GameChatChannel::GameChatChannel(uint32_t id) :
    ChatChannel(id)
{
    game_ = GameManager::Instance.Get(id);
}

bool GameChatChannel::Talk(Player* player, const std::string& text)
{
    if (auto g = game_.lock())
    {
        Net::NetworkMessage msg;
        const std::map<uint32_t, Player*>& players = g->GetPlayers();
        msg.AddByte(AB::GameProtocol::ChatMessage);
        msg.AddByte(AB::GameProtocol::ChatChannelGeneral);
        msg.Add<uint32_t>(player->id_);
        msg.AddString(player->GetName());
        msg.AddString(text);
        for (auto& p : players)
        {
            p.second->client_->WriteToOutput(msg);
        }
        return true;
    }
    return false;
}

WhisperChatChannel::WhisperChatChannel(uint32_t id) :
    ChatChannel(id)
{
    player_ = PlayerManager::Instance.GetPlayerById(id);
}

bool WhisperChatChannel::Talk(Player* player, const std::string& text)
{
    if (auto p = player_.lock())
    {
        Net::NetworkMessage msg;
        msg.AddByte(AB::GameProtocol::ChatMessage);
        msg.AddByte(AB::GameProtocol::ChatChannelWhisper);
        msg.Add<uint32_t>(player->id_);
        msg.AddString(player->GetName());
        msg.AddString(text);
        p->client_->WriteToOutput(msg);
        return true;
    }
    return false;
}

}
