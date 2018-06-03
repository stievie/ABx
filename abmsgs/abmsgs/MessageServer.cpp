#include "stdafx.h"
#include "MessageServer.h"

MessageServer::MessageServer(asio::io_service& ioService,
    const asio::ip::tcp::endpoint& endpoint) :
    ioService_(ioService),
    acceptor_(ioService, endpoint)
{
    StartAccept();
}

void MessageServer::StartAccept()
{
    std::shared_ptr<MessageSession> session = std::make_shared<MessageSession>(ioService_, channel_);
    acceptor_.async_accept(session->Socket(),
        std::bind(&MessageServer::HandleAccept, this, session, std::placeholders::_1));
}

void MessageServer::HandleAccept(std::shared_ptr<MessageSession> session,
    const asio::error_code& error)
{
    if (!error)
        session->Start();
    StartAccept();
}
