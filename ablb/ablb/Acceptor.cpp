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
