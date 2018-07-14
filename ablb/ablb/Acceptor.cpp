#include "stdafx.h"
#include "Acceptor.h"

void Acceptor::HandleAccept(const std::error_code& error)
{
    if (!error)
    {
        session_->Start(upstreamHost_, upstreamPort_);

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
