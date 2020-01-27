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
#include "Bridge.h"
#include <functional>

void Bridge::HandleUpstreamConnect(const std::error_code& error)
{
    if (error)
    {
        Close();
        return;
    }

    // Setup async read from remote server (upstream)
    upstreamSocket_.async_read_some(
        asio::buffer(upstreamData_, max_data_length),
        std::bind(&Bridge::HandleUpstreamRead,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));

    // Setup async read from client (downstream)
    upstreamSocket_.async_read_some(
        asio::buffer(downstreamData_, max_data_length),
        std::bind(&Bridge::HandleDownstreamRead,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
}

void Bridge::HandleUpstreamRead(const std::error_code& error, const size_t& bytes_transferred)
{
    if (error)
    {
        Close();
        return;
    }

    asio::async_write(downstreamSocket_,
        asio::buffer(upstreamData_, bytes_transferred),
        std::bind(&Bridge::HandleDownstreamWrite,
            shared_from_this(),
            std::placeholders::_1));
}

void Bridge::HandleDownstreamWrite(const std::error_code& error)
{
    if (error)
    {
        Close();
        return;
    }

    upstreamSocket_.async_read_some(
        asio::buffer(upstreamData_, max_data_length),
        std::bind(&Bridge::HandleUpstreamRead,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
}

void Bridge::HandleDownstreamRead(const std::error_code& error, const size_t& bytes_transferred)
{
    if (error)
    {
        Close();
        return;
    }

    asio::async_write(upstreamSocket_,
        asio::buffer(downstreamData_, bytes_transferred),
        std::bind(&Bridge::HandleUpstreamWrite,
            shared_from_this(),
            std::placeholders::_1));
}

void Bridge::HandleUpstreamWrite(const std::error_code& error)
{
    if (error)
    {
        Close();
        return;
    }

    downstreamSocket_.async_read_some(
        asio::buffer(downstreamData_, max_data_length),
        std::bind(&Bridge::HandleDownstreamRead,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
}

void Bridge::Close()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (downstreamSocket_.is_open())
        downstreamSocket_.close();
    if (upstreamSocket_.is_open())
        upstreamSocket_.close();
}

void Bridge::Start(const std::string& upstreamHost, uint16_t unstreamPort)
{
    // Connect to remote server, upstream
    upstreamSocket_.async_connect(
        asio::ip::tcp::endpoint(
            asio::ip::address::from_string(upstreamHost),
            unstreamPort
        ),
        std::bind(&Bridge::HandleUpstreamConnect, shared_from_this(), std::placeholders::_1)
    );
}
