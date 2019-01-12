#pragma once

#include "MessageChannel.h"
#include "IpList.h"

class MessageServer
{
private:
    asio::io_service& ioService_;
    asio::ip::tcp::acceptor acceptor_;
    MessageChannel channel_;
    Net::IpList& whiteList_;
    void StartAccept();
    void HandleAccept(std::shared_ptr<MessageSession> session,
        const asio::error_code& error);
    bool IsIpAllowed(const asio::ip::tcp::endpoint& ep);
public:
    MessageServer(asio::io_service& ioService,
        const asio::ip::tcp::endpoint& endpoint,
        Net::IpList& whiteList);
    ~MessageServer() = default;
};

