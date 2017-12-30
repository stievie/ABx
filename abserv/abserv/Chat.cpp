#include "stdafx.h"
#include "Chat.h"
#include "Game.h"
#include "Player.h"

namespace Game {

Chat Chat::Instance;

ChatChannel* Chat::Get(uint8_t type, uint32_t id)
{
    uint64_t channelId = ((uint64_t)type << 32) | id;
    auto it = channels_.find(channelId);
    if (it != channels_.end())
        return (*it).second.get();

    std::unique_ptr<ChatChannel> c;
    if (type == ChannelMap || type == ChannelTrade)
        c = std::make_unique<GameChatChannel>();
    else
        c = std::make_unique<ChatChannel>();
    ChatChannel* result = c.get();
    channels_.emplace(channelId, std::move(c));
    return result;
}

void Chat::Remove(uint8_t type, uint32_t id)
{
    uint64_t channelId = ((uint64_t)type << 32) | id;
    auto it = channels_.find(channelId);
    if (it != channels_.end())
        channels_.erase(it);
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

}
