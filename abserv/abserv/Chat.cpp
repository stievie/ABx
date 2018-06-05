#include "stdafx.h"
#include "Chat.h"
#include "Game.h"
#include "Player.h"
#include "GameManager.h"
#include "PlayerManager.h"
#include "StringHash.h"
#include "Application.h"
#include "MessageMsg.h"
#include <AB/Entities/GuildMembers.h>
#include "DataClient.h"
#include <uuid.h>

namespace Game {

Chat Chat::Instance;

std::shared_ptr<ChatChannel> Chat::Get(uint8_t type, uint64_t id)
{
    if (type == ChannelWhisper)
    {
        return std::make_shared<WhisperChatChannel>(id);
    }

    std::pair<uint8_t, uint64_t> channelId = { type, id };
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

std::shared_ptr<ChatChannel> Chat::Get(uint8_t type, const std::string uuid)
{
    if (uuid.empty() || uuids::uuid(uuid).nil())
        return std::shared_ptr<ChatChannel>();

    std::pair<uint8_t, uint64_t> channelId = { type, Utils::StringHash(uuid.c_str()) };
    auto it = channels_.find(channelId);
    if (it != channels_.end())
        return (*it).second;

    std::shared_ptr<ChatChannel> c;
    switch (type)
    {
    case ChannelGuild:
        c = std::make_shared<GuildChatChannel>(uuid);
        channels_.emplace(channelId, c);
        break;
    case ChannelWhisper:
        c = std::make_shared<WhisperChatChannel>(uuid);
        channels_.emplace(channelId, c);
    }
    return c;
}

void Chat::Remove(uint8_t type, uint64_t id)
{
    std::pair<uint8_t, uint64_t> channelId = { type, id };
    auto it = channels_.find(channelId);
    if (it != channels_.end())
        channels_.erase(it);
}

void Chat::CleanChats()
{
    if (channels_.size() == 0)
        return;

#ifdef _DEBUG
    LOG_DEBUG << "Cleaning chats" << std::endl;
#endif
    auto i = channels_.begin();
    while ((i = std::find_if(i, channels_.end(), [](const auto& current) -> bool
    {
        return current.second.use_count() == 1;
    })) != channels_.end())
        channels_.erase(i++);
}

GameChatChannel::GameChatChannel(uint64_t id) :
    ChatChannel(id)
{
    game_ = GameManager::Instance.Get(static_cast<uint32_t>(id));
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

WhisperChatChannel::WhisperChatChannel(uint64_t id) :
    ChatChannel(id)
{
    player_ = PlayerManager::Instance.GetPlayerById(static_cast<uint32_t>(id));
}

WhisperChatChannel::WhisperChatChannel(const std::string& playerUuid) :
    ChatChannel(Utils::StringHashRt(playerUuid.c_str())),
    playerUuid_(playerUuid)
{
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

    // Maybe not on this server
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::Whipser;
    Net::MessageClient* cli = Application::Instance->GetMessageClient();
    std::stringstream ss;
    ss << playerUuid_ << "|" << player->GetName() << ":";
    ss << text;
    msg.SetBodyString(ss.str());
    cli->Write(msg);
    return true;
}

bool WhisperChatChannel::Talk(const std::string& playerName, const std::string& text)
{
    if (auto p = player_.lock())
    {
        Net::NetworkMessage msg;
        msg.AddByte(AB::GameProtocol::ChatMessage);
        msg.AddByte(AB::GameProtocol::ChatChannelWhisper);
        msg.Add<uint32_t>(0);
        msg.AddString(playerName);
        msg.AddString(text);
        p->client_->WriteToOutput(msg);
        return true;
    }
    return false;
}

GuildChatChannel::GuildChatChannel(const std::string& guildUuid) :
    ChatChannel(Utils::StringHashRt(guildUuid.c_str())),
    guildUuid_(guildUuid)
{
}

bool GuildChatChannel::Talk(Player* player, const std::string& text)
{
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::GuildChat;
    Net::MessageClient* cli = Application::Instance->GetMessageClient();
    std::stringstream ss;
    ss << guildUuid_ << "|" << player->GetName() << ":";
    ss << text;
    msg.SetBodyString(ss.str());
    cli->Write(msg);
    return true;
}

void GuildChatChannel::Broadcast(const std::string& playerName, const std::string& text)
{
    IO::DataClient* cli = Application::Instance->GetDataClient();
    AB::Entities::GuildMembers gs;
    gs.uuid = guildUuid_;
    if (!cli->Read(gs))
        return;

    for (const auto& g : gs.members)
    {
        std::shared_ptr<Player> player = PlayerManager::Instance.GetPlayerByAccountUuid(g.accountUuid);
        if (!player)
            continue;

        Net::NetworkMessage msg;
        msg.AddByte(AB::GameProtocol::ChatMessage);
        msg.AddByte(AB::GameProtocol::ChatChannelGuild);
        msg.Add<uint32_t>(0);
        msg.AddString(playerName);
        msg.AddString(text);
        player->client_->WriteToOutput(msg);
    }
}

}
