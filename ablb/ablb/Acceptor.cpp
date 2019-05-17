#include "stdafx.h"
#include "Acceptor.h"
#include <functional>
#include "Subsystems.h"
#include "BanManager.h"

void Acceptor::HandleAccept(const std::error_code& error)
{
    if (!error)
    {
        auto& socket = session_->GetDownstreamSocket();
        auto ep = socket.remote_endpoint();
        auto ip = ep.address().to_v4().to_uint();
        if (!GetSubsystem<Auth::BanManager>()->AcceptConnection(ip))
        {
            socket.shutdown(asio::ip::tcp::socket::shutdown_both);
            socket.close();
        }
        else
        {
            AB::Entities::Service svc;
            if (getServiceCallback_(svc))
                session_->Start(svc.host, svc.port);
        }

        // Accept more connections
        if (!AcceptConnections())
        {
            LOG_ERROR << "Failed to accept connections" << std::endl;
        }
    }
    else
        LOG_ERROR << error.message() << std::endl;
}

bool Acceptor::AcceptConnections()
{
    try
    {
        session_ = std::make_shared<Bridge>(ioService_);

        acceptor_.async_accept(session_->GetDownstreamSocket(),
            std::bind(&Acceptor::HandleAccept,
                this,
                std::placeholders::_1));
    }
    catch (std::exception& e)
    {
        LOG_ERROR << "Acceptor exception: " << e.what() << std::endl;
        return false;
    }

    return true;
}
