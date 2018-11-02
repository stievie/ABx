#include "stdafx.h"
#include "MessageDispatcher.h"
#include "Chat.h"
#include "StringUtils.h"
#include "PlayerManager.h"
#include "Player.h"
#include "PropStream.h"
#include <AB/ProtocolCodes.h>
#include "Subsystems.h"

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
        std::dynamic_pointer_cast<Game::GuildChatChannel>(GetSubsystem<Game::Chat>()->Get(Game::ChatType::Guild, guildUuid));
    if (chat)
        chat->Broadcast(name, message);
}

void MessageDispatcher::DispatchTradeChat(const Net::MessageMsg& msg)
{
    IO::PropReadStream stream;
    if (!msg.GetPropStream(stream))
        return;
    std::string name;
    if (!stream.ReadString(name))
        return;
    std::string message;
    if (!stream.ReadString(message))
        return;

    std::shared_ptr<Game::TradeChatChannel> chat =
        std::dynamic_pointer_cast<Game::TradeChatChannel>(GetSubsystem<Game::Chat>()->Get(Game::ChatType::Trade, 0));
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

    std::shared_ptr<Game::Player> player = GetSubsystem<Game::PlayerManager>()->GetPlayerByUuid(playerUuid);
    if (!player)
        return;

    std::shared_ptr<Game::WhisperChatChannel> chat =
        std::dynamic_pointer_cast<Game::WhisperChatChannel>(GetSubsystem<Game::Chat>()->Get(Game::ChatType::Whisper, player->id_));
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
    std::shared_ptr<Game::Player> player = GetSubsystem<Game::PlayerManager>()->GetPlayerByAccountUuid(recvAccUuid);
    if (!player)
        return;
    player->NotifyNewMail();
}

void MessageDispatcher::DispatchServerChange(const Net::MessageMsg& msg)
{
    Net::NetworkMessage nmsg;
    IO::PropReadStream prop;
    if (!msg.GetPropStream(prop))
        return;

    AB::Entities::ServiceType t;
    prop.Read<AB::Entities::ServiceType>(t);
    std::string serverId;
    prop.ReadString(serverId);
    if (t != AB::Entities::ServiceTypeGameServer)
        return;

    if (msg.type_ == Net::MessageType::ServerJoined)
        nmsg.AddByte(AB::GameProtocol::ServerJoined);
    else
        nmsg.AddByte(AB::GameProtocol::ServerLeft);

    nmsg.AddString(serverId);    // Server ID
    GetSubsystem<Game::PlayerManager>()->BroadcastNetMessage(nmsg);
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
    case Net::MessageType::TradeChat:
        DispatchTradeChat(msg);
        break;
    case Net::MessageType::ServerJoined:
    case Net::MessageType::ServerLeft:
        DispatchServerChange(msg);
        break;
    }
}
