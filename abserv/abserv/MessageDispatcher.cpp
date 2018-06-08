#include "stdafx.h"
#include "MessageDispatcher.h"
#include "Chat.h"
#include "StringUtils.h"
#include "PlayerManager.h"
#include "Player.h"
#include "PropStream.h"

void MessageDispatcher::DispatchGuildChat(const Net::MessageMsg& msg)
{
    IO::PropReadStream stream;
    if (!msg.GetPropStream(stream))
        return;

    std::string guildUuid;
    if (!stream.ReadString(guildUuid))
        return;
    std::string name;
    if (!stream.ReadString(name))
        return;
    std::string message;
    if (!stream.ReadString(message))
        return;

    std::shared_ptr<Game::GuildChatChannel> chat =
        std::dynamic_pointer_cast<Game::GuildChatChannel>(Game::Chat::Instance.Get(Game::ChannelGuild, guildUuid));
    if (chat)
        chat->Broadcast(name, message);
}

void MessageDispatcher::DispatchWhipserChat(const Net::MessageMsg& msg)
{
    IO::PropReadStream stream;
    if (!msg.GetPropStream(stream))
        return;

    std::string playerUuid;
    if (!stream.ReadString(playerUuid))
        return;
    std::string name;
    if (!stream.ReadString(name))
        return;
    std::string message;
    if (!stream.ReadString(message))
        return;

    std::shared_ptr<Game::Player> player = Game::PlayerManager::Instance.GetPlayerByUuid(playerUuid);
    if (!player)
        return;

    std::shared_ptr<Game::WhisperChatChannel> chat =
        std::dynamic_pointer_cast<Game::WhisperChatChannel>(Game::Chat::Instance.Get(Game::ChannelWhisper, player->id_));
    if (chat)
        chat->Talk(name, message);
}

void MessageDispatcher::DispatchNewMail(const Net::MessageMsg& msg)
{
    IO::PropReadStream stream;
    if (!msg.GetPropStream(stream))
        return;

    std::string recvAccUuid;
    if (!stream.ReadString(recvAccUuid))
        return;
    std::shared_ptr<Game::Player> player = Game::PlayerManager::Instance.GetPlayerByAccountUuid(recvAccUuid);
    if (!player)
        return;
    player->NotifyNewMail();
}

void MessageDispatcher::Dispatch(const Net::MessageMsg& msg)
{
    switch (msg.type_)
    {
    case Net::MessageType::GuildChat:
        DispatchGuildChat(msg);
        break;
    case Net::MessageType::Whipser:
        DispatchWhipserChat(msg);
        break;
    case Net::MessageType::NewMail:
        DispatchNewMail(msg);
        break;
    }
}
