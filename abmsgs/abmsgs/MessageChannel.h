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

#pragma once

#include <memory>
#include <asio.hpp>
#include <abscommon/MessageMsg.h>
#include <set>
#include <AB/Entities/Service.h>

class MessageParticipant
{
public:
    virtual ~MessageParticipant();
    virtual void Deliver(const Net::MessageMsg& msg) = 0;
    std::string serverId_;
    AB::Entities::ServiceType serviceType_;
};

class MessageChannel
{
private:
    enum { MaxRecentMsgs = 100 };
    Net::MessageQueue recentMsgs_;
public:
    void Join(std::shared_ptr<MessageParticipant> participant);
    void Leave(std::shared_ptr<MessageParticipant> participant);
    void Deliver(const Net::MessageMsg& msg);
    std::set<std::shared_ptr<MessageParticipant>> participants_;
};

class MessageSession : public MessageParticipant, public std::enable_shared_from_this<MessageSession>
{
private:
    asio::ip::tcp::socket socket_;
    asio::io_service::strand strand_;
    MessageChannel& channel_;
    Net::MessageMsg readMsg_;
    Net::MessageQueue writeMsgs_;
    void HandleRead(const asio::error_code& error);
    void HandleWrite(const asio::error_code& error, size_t);
    void HandleMessage(const Net::MessageMsg& msg);
    void HandleWhisperMessage(const Net::MessageMsg& msg);
    void HandleNewMailMessage(const Net::MessageMsg& msg);
    void HandleQueueMessage(const Net::MessageMsg& msg);
    void HandlePlayerChangedMessage(const Net::MessageMsg& msg);
    void HandleQueuePlayerMessage(const Net::MessageMsg& msg);
    void HandleQueueTeamEnterMessage(const Net::MessageMsg& msg);
    void SendPlayerMessage(const std::string& playerUuid, const Net::MessageMsg& msg);
    MessageParticipant* GetServerWidthPlayer(const std::string& playerUuid);
    std::string GetServerUuidWidthAccount(const std::string& accountUuid);
    MessageParticipant* GetServerWidthAccount(const std::string& accountUuid);
    MessageParticipant* GetServer(const std::string& serverUuid);
    MessageParticipant* GetServerByType(AB::Entities::ServiceType type);
    void WriteImpl(const Net::MessageMsg& msg);
    void Write();
public:
    MessageSession(asio::io_service& io_service, MessageChannel& channel) :
        socket_(io_service),
        strand_(io_service),
        channel_(channel)
    { }

    asio::ip::tcp::socket& Socket()
    {
        return socket_;
    }

    void Start();
    void Deliver(const Net::MessageMsg& msg) override;
};
