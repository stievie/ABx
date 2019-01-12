#include "stdafx.h"
#include "MessageServer.h"

MessageServer::MessageServer(asio::io_service& ioService,
    const asio::ip::tcp::endpoint& endpoint, Net::IpList& whiteList) :
    ioService_(ioService),
    acceptor_(ioService, endpoint),
    whiteList_(whiteList)
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
    {
        auto endp = session->Socket().remote_endpoint();
        if (IsIpAllowed(endp))
            session->Start();
        else
            LOG_ERROR << "Connection from " << endp.address() << " not allowed" << std::endl;
    }
    StartAccept();
}

bool MessageServer::IsIpAllowed(const asio::ip::tcp::endpoint& ep)
{
    if (whiteList_.IsEmpty())
        return true;
    return whiteList_.Contains(ep.address().to_v4().to_uint());
}
