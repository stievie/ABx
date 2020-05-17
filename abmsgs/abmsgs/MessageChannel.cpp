/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include "MessageChannel.h"
#include "Application.h"
#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/FriendList.h>
#include <AB/Entities/FriendedMe.h>
#include <AB/Entities/GuildMembers.h>
#include <abscommon/Logger.h>
#include <sa/PropStream.h>
#include <abscommon/Subsystems.h>
#include <abscommon/UuidUtils.h>
#include <abscommon/UuidUtils.h>
#include <functional>

MessageParticipant::~MessageParticipant() = default;

void MessageChannel::Join(std::shared_ptr<MessageParticipant> participant)
{
    participants_.insert(participant);
    ea::for_each(recentMsgs_.begin(), recentMsgs_.end(),
        std::bind(&MessageParticipant::Deliver, participant, std::placeholders::_1));
}

void MessageChannel::Leave(std::shared_ptr<MessageParticipant> participant)
{
    participants_.erase(participant);
}

void MessageChannel::Deliver(const Net::MessageMsg& msg)
{
    recentMsgs_.push_back(msg);
    while (recentMsgs_.size() > MaxRecentMsgs)
        recentMsgs_.pop_front();
    ea::for_each(participants_.begin(), participants_.end(),
        std::bind(&MessageParticipant::Deliver, std::placeholders::_1, msg));
}

void MessageSession::HandleRead(const asio::error_code& error)
{
    if (!error && readMsg_.DecodeHeader())
    {
        asio::read(socket_,
            asio::buffer(readMsg_.Body(), readMsg_.BodyLength()));
        HandleMessage(readMsg_);

        asio::async_read(socket_,
            asio::buffer(readMsg_.Data(), Net::MessageMsg::HeaderLength),
            std::bind(
                &MessageSession::HandleRead, shared_from_this(), std::placeholders::_1
            ));
    }
    else
    {
        LOG_ERROR << "(" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
        channel_.Leave(shared_from_this());
    }
}

void MessageSession::HandleMessage(const Net::MessageMsg& msg)
{
    switch (msg.type_)
    {
    case Net::MessageType::Unknown:
        LOG_WARNING << "Unknown message type: " << static_cast<int>(msg.type_) << std::endl;
        break;
    case Net::MessageType::ServerJoined:
    {
        sa::PropReadStream prop;
        if (msg.GetPropStream(prop))
        {
            // Get type and UUID of server
            prop.Read<AB::Entities::ServiceType>(serviceType_);
            prop.ReadString(serverId_);
            channel_.Deliver(msg);
        }
        break;
    }
    case Net::MessageType::ServerLeft:
        // Server was shut down
        channel_.Deliver(msg);
        break;
    case Net::MessageType::Shutdown:
    case Net::MessageType::Spawn:
    case Net::MessageType::ClearCache:
    {
        // Send shutdown message to server
        std::string serverId = msg.GetBodyString();
        if (!serverId.empty())
        {
            MessageParticipant* server = GetServer(serverId);
            if (server)
                server->Deliver(msg);
        }
        break;
    }
    case Net::MessageType::Whipser:
        // Alternatively we could just use channel_.Deliver(msg)
        HandleWhisperMessage(msg);
        break;
    case Net::MessageType::NewMail:
        HandleNewMailMessage(msg);
        break;
    case Net::MessageType::PlayerChanged:
        HandlePlayerChangedMessage(msg);
        break;
    case Net::MessageType::QueueAdd:
    case Net::MessageType::QueueRemove:
        HandleQueueMessage(msg);
        break;
    case Net::MessageType::PlayerAddedToQueue:
    case Net::MessageType::PlayerRemovedFromQueue:
        HandleQueuePlayerMessage(msg);
        break;
    case Net::MessageType::TeamsEnterMatch:
        HandleQueueTeamEnterMessage(msg);
        break;
    default:
        channel_.Deliver(msg);
        break;
    }
}

void MessageSession::SendPlayerMessage(const std::string& playerUuid, const Net::MessageMsg& msg)
{
    auto* server = GetServerWidthPlayer(playerUuid);
    if (server)
        server->Deliver(msg);
}

void MessageSession::HandleQueueTeamEnterMessage(const Net::MessageMsg& msg)
{
    std::string queueUuid;
    std::string mapUuid;
    std::string instanceUuid;
    std::string hostingServer;
    sa::PropReadStream stream;
    if (!msg.GetPropStream(stream))
        return;
    if (!stream.ReadString(queueUuid))
        return;
    if (!stream.ReadString(hostingServer))
        return;
    if (!stream.ReadString(mapUuid))
        return;
    if (!stream.ReadString(instanceUuid))
        return;

    Net::MessageMsg createInstMsg;
    createInstMsg.type_ = Net::MessageType::CreateGameInstance;
    sa::PropWriteStream createInstStream;
    createInstStream.WriteString(hostingServer);
    createInstStream.WriteString(mapUuid);
    createInstStream.WriteString(instanceUuid);

    // Send the hosting server a message to create the instance
    auto* server = GetServer(hostingServer);
    if (!server)
    {
        LOG_ERROR << "Server not found " << hostingServer << std::endl;
        return;
    }
    server->Deliver(createInstMsg);

    // This message must gell all server that have members. The hosting server is responsible to create the instance
    uint8_t numTeams{ 0 };
    stream.Read<uint8_t>(numTeams);
    for (uint8_t t = 0; t < numTeams; ++t)
    {
        uint8_t numMewmbers{ 0 };
        stream.Read<uint8_t>(numMewmbers);
        for (uint8_t m = 0; m < numMewmbers; ++m)
        {
            std::string member;
            if (!stream.ReadString(member))
                continue;
            // Send this message to all players on their current game server
            // This message also contains the hosting game server
            SendPlayerMessage(member, msg);
        }
    }
}

void MessageSession::HandleQueueMessage(const Net::MessageMsg& msg)
{
    auto* server = GetServerByType(AB::Entities::ServiceTypeMatchServer);
    if (server)
        server->Deliver(msg);
    else
        LOG_WARNING << "Match making server not running" << std::endl;
}

void MessageSession::HandlePlayerChangedMessage(const Net::MessageMsg& msg)
{
    sa::PropReadStream stream;
    if (!msg.GetPropStream(stream))
    {
        LOG_ERROR << "Unable to get property stream" << std::endl;
        return;
    }
    uint32_t fields{ 0 };
    if (!stream.Read<uint32_t>(fields))
    {
        LOG_ERROR << "Unable to fields" << std::endl;
        return;
    }
    if (fields == 0)
    {
        LOG_ERROR << "fields == 0" << std::endl;
        return;
    }
    std::string accountUuid;
    if (!stream.ReadString(accountUuid))
    {
        LOG_ERROR << "Unable to read accountt UUID" << std::endl;
        return;
    }

    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Account acc;
    acc.uuid = accountUuid;
    if (!client->Read(acc))
    {
        LOG_ERROR << "Unable to read account with UUID " << acc.uuid << std::endl;
        return;
    }

    std::vector<std::string> interested;
    std::unordered_set<std::string> ignored;

    // I friended those
    AB::Entities::FriendList fl;
    fl.uuid = accountUuid;
    if (client->Read(fl))
    {
        for (const auto& f : fl.friends)
        {
            if (f.relation == AB::Entities::FriendRelationFriend)
                interested.push_back(f.friendUuid);
            else if (f.relation == AB::Entities::FriendRelationIgnore)
                ignored.emplace(f.friendUuid);
        }
    }
    // Those friended me
    AB::Entities::FriendedMe fme;
    fme.uuid = accountUuid;
    if (client->Read(fme))
    {
        for (const auto& f : fme.friends)
        {
            if (f.relation == AB::Entities::FriendRelationFriend)
                interested.push_back(f.accountUuid);
            else if (f.relation == AB::Entities::FriendRelationIgnore)
                ignored.emplace(f.accountUuid);
        }
    }

    auto isIgnored = [&ignored](const std::string& uuid)
    {
        const auto it = ignored.find(uuid);
        return it != ignored.end();
    };

    if (!Utils::Uuid::IsEmpty(acc.guildUuid))
    {
        // If this guy is in a guild also inform guild members
        AB::Entities::GuildMembers gms;
        gms.uuid = acc.guildUuid;
        if (client->Read(gms))
        {
            for (const auto& gm : gms.members)
            {
                if (!Utils::Uuid::IsEqual(accountUuid, gm.accountUuid) && !isIgnored(gm.accountUuid))
                    // Don't add self and ignored
                    interested.push_back(gm.accountUuid);
            }
        }
    }
    std::sort(interested.begin(), interested.end());
    interested.erase(std::unique(interested.begin(), interested.end()), interested.end());

    // Get all servers
    std::vector<std::string> servers;
    for (const auto& informAcc : interested)
    {
        std::string serverUuid = GetServerUuidWidthAccount(informAcc);
        if (!Utils::Uuid::IsEmpty(serverUuid))
            servers.push_back(serverUuid);
    }

    // Get unique servers
    std::sort(servers.begin(), servers.end());
    servers.erase(std::unique(servers.begin(), servers.end()), servers.end());
    for (const auto& serverUuid : servers)
    {
        auto* server = GetServer(serverUuid);
        if (server)
            server->Deliver(msg);
    }
}

void MessageSession::HandleQueuePlayerMessage(const Net::MessageMsg& msg)
{
    std::string playerUuid;
    sa::PropReadStream stream;
    if (!msg.GetPropStream(stream))
        return;
    if (!stream.ReadString(playerUuid))
        return;

    SendPlayerMessage(playerUuid, msg);
}

void MessageSession::HandleWhisperMessage(const Net::MessageMsg& msg)
{
    std::string receiverUuid;
    sa::PropReadStream stream;
    if (!msg.GetPropStream(stream))
        return;
    if (!stream.ReadString(receiverUuid))
        return;

    SendPlayerMessage(receiverUuid, msg);
}

void MessageSession::HandleNewMailMessage(const Net::MessageMsg& msg)
{
    std::string recvAccUuid;
    sa::PropReadStream stream;
    if (!msg.GetPropStream(stream))
        return;
    if (!stream.ReadString(recvAccUuid))
        return;

    MessageParticipant* server = GetServerWidthAccount(recvAccUuid);
    if (server)
    {
        server->Deliver(msg);
    }
}

MessageParticipant* MessageSession::GetServerWidthPlayer(const std::string& playerUuid)
{
    IO::DataClient* cli = GetSubsystem<IO::DataClient>();
    // Find the server this player is on
    AB::Entities::Character ch;
    ch.uuid = playerUuid;
    if (!cli->Read(ch))
        return nullptr;
    return GetServerWidthAccount(ch.accountUuid);
}

std::string MessageSession::GetServerUuidWidthAccount(const std::string& accountUuid)
{
    IO::DataClient* cli = GetSubsystem<IO::DataClient>();
    AB::Entities::Account acc;
    acc.uuid = accountUuid;
    if (!cli->Read(acc))
        return Utils::Uuid::EMPTY_UUID;
    return acc.currentServerUuid;
}

MessageParticipant* MessageSession::GetServerWidthAccount(const std::string& accountUuid)
{
    return GetServer(GetServerUuidWidthAccount(accountUuid));
}

MessageParticipant* MessageSession::GetServerByType(AB::Entities::ServiceType type)
{
    auto serv = ea::find_if(channel_.participants_.begin(),
        channel_.participants_.end(), [type](const std::shared_ptr<MessageParticipant>& current)
    {
        return current->serviceType_ == type;
    });
    if (serv != channel_.participants_.end())
        return (*serv).get();
    return nullptr;
}

MessageParticipant* MessageSession::GetServer(const std::string& serverUuid)
{
    auto serv = ea::find_if(channel_.participants_.begin(),
        channel_.participants_.end(), [&serverUuid](const std::shared_ptr<MessageParticipant>& current)
    {
        return Utils::Uuid::IsEqual(current->serverId_, serverUuid);
    });
    if (serv != channel_.participants_.end())
        return (*serv).get();
    return nullptr;
}

void MessageSession::Start()
{
    auto endp = socket_.remote_endpoint();
    channel_.Join(shared_from_this());
    asio::async_read(socket_,
        asio::buffer(readMsg_.Data(), Net::MessageMsg::HeaderLength),
        std::bind(
            &MessageSession::HandleRead, shared_from_this(), std::placeholders::_1
        ));
}

void MessageSession::HandleWrite(const asio::error_code& error, size_t)
{
    if (!error)
    {
        writeMsgs_.pop_front();
        if (!writeMsgs_.empty())
            Write();
    }
    else
    {
        LOG_ERROR << "(" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
        channel_.Leave(shared_from_this());
    }
}

void MessageSession::WriteImpl(const Net::MessageMsg& msg)
{
    writeMsgs_.push_back(msg);
    if (writeMsgs_.size() > 1)
        // Write in progress
        return;
    Write();
}

void MessageSession::Write()
{
    const Net::MessageMsg& msg = writeMsgs_[0];
    asio::async_write(
        socket_,
        asio::buffer(msg.Data(), msg.Length()),
        strand_.wrap(
            std::bind(
                &MessageSession::HandleWrite,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2
            )
        )
    );
}

void MessageSession::Deliver(const Net::MessageMsg& msg)
{
    strand_.post(std::bind(&MessageSession::WriteImpl, shared_from_this(), msg));
}
