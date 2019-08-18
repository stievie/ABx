#include "stdafx.h"
#include "MessageChannel.h"
#include <functional>
#include "Logger.h"
#include "Application.h"
#include "PropStream.h"
#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include "Subsystems.h"
#include "UuidUtils.h"

MessageParticipant::~MessageParticipant() = default;

void MessageChannel::Join(std::shared_ptr<MessageParticipant> participant)
{
    participants_.insert(participant);
    std::for_each(recentMsgs_.begin(), recentMsgs_.end(),
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
    std::for_each(participants_.begin(), participants_.end(),
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

void MessageSession::HandleWrite(const asio::error_code& error)
{
    if (!error)
    {
        writeMsgs_.pop_front();
        if (!writeMsgs_.empty())
        {
            asio::async_write(socket_,
                asio::buffer(writeMsgs_.front().Data(), writeMsgs_.front().Length()),
                std::bind(&MessageSession::HandleWrite, shared_from_this(), std::placeholders::_1));
        }
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
        IO::PropReadStream prop;
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
    IO::PropReadStream stream;
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
    IO::PropWriteStream createInstStream;
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

void MessageSession::HandleQueuePlayerMessage(const Net::MessageMsg& msg)
{
    std::string playerUuid;
    IO::PropReadStream stream;
    if (!msg.GetPropStream(stream))
        return;
    if (!stream.ReadString(playerUuid))
        return;

    SendPlayerMessage(playerUuid, msg);
}

void MessageSession::HandleWhisperMessage(const Net::MessageMsg& msg)
{
    std::string receiverUuid;
    IO::PropReadStream stream;
    if (!msg.GetPropStream(stream))
        return;
    if (!stream.ReadString(receiverUuid))
        return;

    SendPlayerMessage(receiverUuid, msg);
}

void MessageSession::HandleNewMailMessage(const Net::MessageMsg& msg)
{
    std::string recvAccUuid;
    IO::PropReadStream stream;
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

MessageParticipant* MessageSession::GetServerWidthAccount(const std::string& accountUuid)
{
    // No need to send this message to all servers
    IO::DataClient* cli = GetSubsystem<IO::DataClient>();
    AB::Entities::Account acc;
    acc.uuid = accountUuid;
    if (!cli->Read(acc))
        return nullptr;

    return GetServer(acc.currentServerUuid);
}

MessageParticipant* MessageSession::GetServerByType(AB::Entities::ServiceType type)
{
    auto serv = std::find_if(channel_.participants_.begin(),
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
    auto serv = std::find_if(channel_.participants_.begin(),
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
    LOG_INFO << "Connection from " << endp.address() << ":" << endp.port() << std::endl;
    channel_.Join(shared_from_this());
    asio::async_read(socket_,
        asio::buffer(readMsg_.Data(), Net::MessageMsg::HeaderLength),
        std::bind(
            &MessageSession::HandleRead, shared_from_this(), std::placeholders::_1
        ));
}

void MessageSession::Deliver(const Net::MessageMsg& msg)
{
    bool writeInProgress = !writeMsgs_.empty();
    writeMsgs_.push_back(msg);
    if (!writeInProgress)
    {
        asio::async_write(socket_,
            asio::buffer(writeMsgs_.front().Data(), writeMsgs_.front().Length()),
            std::bind(&MessageSession::HandleWrite, shared_from_this(), std::placeholders::_1));
    }
}
