/**
 * Copyright 2020 Stefan Ascher
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

#include "PingServer.h"
#include "Logger.h"

namespace Net {

PingServer::PingServer() :
    ioService_(std::make_shared<asio::io_service>()),
    ownService_(true)
{
}

PingServer::PingServer(std::shared_ptr<asio::io_service> ioService) :
    ioService_(ioService),
    ownService_(false)
{
}

void PingServer::Stop()
{
    running_ = false;
    thread_.detach();
}

void PingServer::Start()
{
    running_ = true;
    thread_ = std::thread(&PingServer::ThreadFunc, this);
}

void PingServer::ThreadFunc()
{
    asio::ip::udp::socket socket{ *ioService_, asio::ip::udp::endpoint(asio::ip::udp::v4(), port_) };

    char data[64];
    asio::ip::udp::endpoint senderEndpoint;
    while (running_)
    {
        asio::error_code ec;
        socket.receive_from(asio::buffer(data, 64), senderEndpoint, 0, ec);
        if (!ec)
        {
            asio::error_code ec2;
            socket.send_to(asio::buffer(data, 64), senderEndpoint, 0, ec2);
            (void)ec2;
        }
    }
    socket.close();
}

}
