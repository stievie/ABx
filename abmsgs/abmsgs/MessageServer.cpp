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

#include "MessageServer.h"
#include <abscommon/StringUtils.h>

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
        {
            LOG_INFO << "Connection from " << endp.address() << ":" << endp.port() << std::endl;
            session->Start();
        }
    }
    StartAccept();
}

bool MessageServer::IsIpAllowed(const asio::ip::tcp::endpoint& ep)
{
    if (whiteList_.IsEmpty())
        return true;
    const uint32_t ip = ep.address().to_v4().to_uint();
    const bool result = whiteList_.Contains(ip);
    if (!result)
        LOG_WARNING << "Connection attempt from a not allowed IP " << Utils::ConvertIPToString(ip) << std::endl;
    return result;
}
