#pragma once

#include "MessageMsg.h"
#include <set>

class MessageParticipant
{
public:
    virtual ~MessageParticipant() { }
    virtual void Deliver(const Net::MessageMsg& msg) = 0;
    std::string serverId_;
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
    MessageChannel& channel_;
    Net::MessageMsg readMsg_;
    Net::MessageQueue writeMsgs_;
    void HandleRead(const asio::error_code& error);
    void HandleWrite(const asio::error_code& error);
    void HandleMessage(const Net::MessageMsg& msg);
    void HandleWhisperMessage(const Net::MessageMsg& msg);
    void HandleNewMailMessage(const Net::MessageMsg& msg);
    MessageParticipant* GetServerWidthPlayer(const std::string& playerUuid);
    MessageParticipant* GetServerWidthAccount(const std::string& accountUuid);
    MessageParticipant* GetServer(const std::string& serverUuid);
public:
    MessageSession(asio::io_service& io_service, MessageChannel& channel) :
        socket_(io_service),
        channel_(channel)
    { }

    asio::ip::tcp::socket& Socket()
    {
        return socket_;
    }

    void Start();
    void Deliver(const Net::MessageMsg& msg) override;
};
