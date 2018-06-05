#include "stdafx.h"
#include "MessageDispatcher.h"
#include "Chat.h"
#include "StringUtils.h"
#include "PlayerManager.h"
#include "Player.h"

void MessageDispatcher::DispatchGuildChat(const Net::MessageMsg& msg)
{
    const std::string text = msg.GetBodyString();
    // GuildUUID|PlayerName:Text
    size_t p = text.find('|');
    if (p != std::string::npos)
    {
        size_t p2 = text.find(':');
        const std::string guildUuid = text.substr(0, p);
        if (guildUuid.empty())
            return;

        std::string name;
        std::string message;

        if (p2 == std::string::npos)
            return;

        name = Utils::Trim(text.substr(p + 1, p2 - p - 1));
        message = Utils::Trim(text.substr(p2 + 1, std::string::npos));

        std::shared_ptr<Game::GuildChatChannel> chat =
            std::dynamic_pointer_cast<Game::GuildChatChannel>(Game::Chat::Instance.Get(Game::ChannelGuild, guildUuid));
        if (chat)
            chat->Broadcast(name, message);
    }
}

void MessageDispatcher::DispatchWhipserChat(const Net::MessageMsg& msg)
{
    const std::string text = msg.GetBodyString();
    // PlayerUUID|PlayerName:Text
    size_t p = text.find('|');
    if (p != std::string::npos)
    {
        size_t p2 = text.find(':');
        const std::string playerUuid = text.substr(0, p);
        if (playerUuid.empty())
            return;

        std::string name;
        std::string message;

        if (p2 == std::string::npos)
            return;

        name = Utils::Trim(text.substr(p + 1, p2 - p - 1));
        message = Utils::Trim(text.substr(p2 + 1, std::string::npos));

        std::shared_ptr<Game::Player> player = Game::PlayerManager::Instance.GetPlayerByUuid(playerUuid);
        if (!player)
            return;

        std::shared_ptr<Game::WhisperChatChannel> chat =
            std::dynamic_pointer_cast<Game::WhisperChatChannel>(Game::Chat::Instance.Get(Game::ChannelWhisper, player->id_));
        if (chat)
            chat->Talk(name, message);
    }
}

void MessageDispatcher::Dispatch(const Net::MessageMsg& msg)
{
    switch (msg.type_)
    {
    case Net::MessageTypeGuildChat:
        DispatchGuildChat(msg);
        break;
    case Net::MessageTypeWhipser:
        DispatchWhipserChat(msg);
        break;
    }
}
