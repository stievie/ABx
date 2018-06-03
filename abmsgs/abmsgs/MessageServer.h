#pragma once

#include "MessageChannel.h"

class MessageServer
{
private:
    asio::io_service& ioService_;
    asio::ip::tcp::acceptor acceptor_;
    MessageChannel channel_;
    void StartAccept();
    void HandleAccept(std::shared_ptr<MessageSession> session,
        const asio::error_code& error);
public:
    MessageServer(asio::io_service& ioService,
        const asio::ip::tcp::endpoint& endpoint);
    ~MessageServer() = default;
};

