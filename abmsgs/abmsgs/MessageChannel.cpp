#include "stdafx.h"
#include "MessageChannel.h"
#include <functional>
#include "Logger.h"
#include "Application.h"
#include "PropStream.h"
#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/Service.h>
#include "Subsystems.h"

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
        LOG_ERROR << "(" << error.value() << ") " << error.message() << std::endl;
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
        LOG_ERROR << "(" << error.value() << ") " << error.message() << std::endl;
        channel_.Leave(shared_from_this());
    }
}

void MessageSession::HandleMessage(const Net::MessageMsg& msg)
{
    switch (msg.type_)
    {
    case Net::MessageType::Unknown:
        LOG_WARNING << "Unknown message type" << std::endl;
        break;
    case Net::MessageType::ServerJoined:
    {
        IO::PropReadStream prop;
        if (msg.GetPropStream(prop))
        {
            AB::Entities::ServiceType t;
            prop.Read<AB::Entities::ServiceType>(t);
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
    {
        // Send shutdown message to server
        std::string serverId = msg.GetBodyString();
        if (!serverId.empty())
        {
            MessageParticipant* server = GetServer(serverId);
            if (server)
            {
                LOG_INFO << "Sending shutdown message to server " << serverId << std::endl;
                server->Deliver(msg);
            }
        }
        break;
    }
    case Net::MessageType::Spawn:
    {
        std::string serverId = msg.GetBodyString();
        if (!serverId.empty())
        {
            MessageParticipant* server = GetServer(serverId);
            if (server)
            {
                LOG_INFO << "Sending spawn message to server " << serverId << std::endl;
                server->Deliver(msg);
            }
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
    default:
        channel_.Deliver(msg);
    }
}

void MessageSession::HandleWhisperMessage(const Net::MessageMsg& msg)
{
    std::string receiverUuid;
    IO::PropReadStream stream;
    if (!msg.GetPropStream(stream))
        return;
    if (!stream.ReadString(receiverUuid))
        return;

    MessageParticipant* server = GetServerWidthPlayer(receiverUuid);
    if (server)
    {
        server->Deliver(msg);
    }
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

MessageParticipant* MessageSession::GetServer(const std::string& serverUuid)
{
    auto serv = std::find_if(channel_.participants_.begin(),
        channel_.participants_.end(), [&serverUuid](const std::shared_ptr<MessageParticipant>& current)
    {
        return current->serverId_.compare(serverUuid) == 0;
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
