#include "stdafx.h"
#include "MessageDispatcher.h"
#include "Chat.h"
#include "StringUtils.h"
#include "PlayerManager.h"
#include "Player.h"
#include "PropStream.h"
#include <AB/ProtocolCodes.h>
#include "Subsystems.h"
#include "IOPlayer.h"
#include <AB/Entities/Character.h>
#include "IOAccount.h"
#include <AB/Packets/ServerPackets.h>
#include <AB/Packets/Packet.h>

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

void MessageDispatcher::DispatchPlayerChanged(const Net::MessageMsg& msg)
{
    IO::PropReadStream stream;
    if (!msg.GetPropStream(stream))
    {
        LOG_ERROR << "Unable to get property stream" << std::endl;
        return;
    }
    uint32_t fields{ 0 };
    if (!stream.Read<uint32_t>(fields))
    {
        LOG_ERROR << "Unable read to fields" << std::endl;
        return;
    }
    if (fields == 0)
    {
        LOG_ERROR << "fields == 0" << std::endl;
        return;
    }

    std::string accUuid;
    if (!stream.ReadString(accUuid))
    {
        LOG_ERROR << "Unable to read account UUID" << std::endl;
        return;
    }

    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Account account;
    account.uuid = accUuid;
    if (!client->Read(account))
    {
        LOG_ERROR << "Unable to read account with UUID " << accUuid << std::endl;
        return;
    }

    AB::Entities::Character ch;
    ch.uuid = account.currentCharacterUuid;
    if (!client->Read(ch))
    {
        LOG_ERROR << "Unable to character with UUID " << ch.uuid << std::endl;
        return;
    }

    std::vector<std::string> accounts;
    IO::IOPlayer::GetInterestedParties(accUuid, accounts);

    auto* playerMan = GetSubsystem<Game::PlayerManager>();
    for (const auto& acc : accounts)
    {
        auto player = playerMan->GetPlayerByAccountUuid(acc);
        if (player)
            player->SendPlayerInfo(ch, fields);
    }
}

void MessageDispatcher::DispatchServerChange(const Net::MessageMsg& msg)
{
    auto nmsg = Net::NetworkMessage::GetNew();
    IO::PropReadStream prop;
    if (!msg.GetPropStream(prop))
        return;

    // AB::Packets::Server::ServerLeft is just an "alias" for AB::Packets::Server::ServerJoined
    AB::Packets::Server::ServerJoined packet;
    prop.Read<uint8_t>(packet.type);
    prop.ReadString(packet.uuid);
    prop.ReadString(packet.host);
    prop.Read<uint16_t>(packet.port);
    prop.ReadString(packet.location);
    prop.ReadString(packet.name);
    prop.ReadString(packet.machine);

    if (static_cast<AB::Entities::ServiceType>(packet.type) != AB::Entities::ServiceTypeGameServer)
        return;

    if (msg.type_ == Net::MessageType::ServerJoined)
    {
#ifdef _DEBUG
        LOG_DEBUG << "Sending server joined message" << std::endl;
#endif
        nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerJoined);
    }
    else
    {
#ifdef _DEBUG
        LOG_DEBUG << "Sending server left message" << std::endl;
#endif
        nmsg->AddByte(AB::GameProtocol::ServerPacketType::ServerLeft);
    }

    AB::Packets::Add(packet, *nmsg);
    GetSubsystem<Game::PlayerManager>()->BroadcastNetMessage(*nmsg);
}

void MessageDispatcher::DispatchTeamsEnterMatch(const Net::MessageMsg& msg)
{
    // If it's a party only the leader gets informed of this. So we just can get the
    // party and call Party::ChangeServerInstance()

    IO::PropReadStream prop;
    if (!msg.GetPropStream(prop))
        return;

    std::vector<std::string> players;
    std::string queueUuid;
    prop.ReadString(queueUuid);
    std::string serverUuid;
    prop.ReadString(serverUuid);
    std::string mapUuid;
    prop.ReadString(mapUuid);
    std::string instanceUuid;
    prop.ReadString(instanceUuid);
    uint8_t teamSize{ 0 };
    prop.Read<uint8_t>(teamSize);
    for (uint8_t i = 0; i < teamSize; ++i)
    {
        uint8_t memberCount{ 0 };
        prop.Read<uint8_t>(memberCount);
        for (uint8_t j = 0; j < memberCount; ++j)
        {
            std::string playerUuid;
            prop.ReadString(playerUuid);
            players.push_back(playerUuid);
        }
    }

    auto* playerMngr = GetSubsystem<Game::PlayerManager>();
    for (const auto& player : players)
    {
        // Get player if s/he is on this server
        auto pPlayer = playerMngr->GetPlayerByUuid(player);
        if (pPlayer)
        {
            pPlayer->GetParty()->ChangeServerInstance(serverUuid, mapUuid, instanceUuid);
        }
    }
}

void MessageDispatcher::DispatchQueueAdded(const Net::MessageMsg& msg)
{
    // Notify players they were added to the queue
    // If it's a party the leader gets informed of this.
    // The game server is sesponsible to inform all party memebers

    IO::PropReadStream prop;
    if (!msg.GetPropStream(prop))
        return;

    std::string playerUuid;
    prop.ReadString(playerUuid);

    auto* playerMngr = GetSubsystem<Game::PlayerManager>();
    auto pPlayer = playerMngr->GetPlayerByUuid(playerUuid);
    if (pPlayer)
        pPlayer->GetParty()->NotifyPlayersQueued();
}

void MessageDispatcher::DispatchQueueRemoved(const Net::MessageMsg& msg)
{
    // Notify players they were removed to the queue
    IO::PropReadStream prop;
    if (!msg.GetPropStream(prop))
        return;

    std::string playerUuid;
    prop.ReadString(playerUuid);

    auto* playerMngr = GetSubsystem<Game::PlayerManager>();
    auto pPlayer = playerMngr->GetPlayerByUuid(playerUuid);
    if (pPlayer)
        pPlayer->GetParty()->NotifyPlayersUnqueued();
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
    case Net::MessageType::PlayerChanged:
        DispatchPlayerChanged(msg);
        break;
    case Net::MessageType::TeamsEnterMatch:
        // Is called when the game was started and the players should enter it
        DispatchTeamsEnterMatch(msg);
        break;
    case Net::MessageType::PlayerAddedToQueue:
        DispatchQueueAdded(msg);
        break;
    case Net::MessageType::PlayerRemovedFromQueue:
        DispatchQueueRemoved(msg);
        break;
    default:
        // Not handled here
        break;
    }
}
