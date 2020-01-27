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

#pragma once

#include <mutex>
#include <memory>
#include <asio.hpp>

class Bridge : public std::enable_shared_from_this<Bridge>
{
public:
    using socket_type = asio::ip::tcp::socket;
private:
    std::mutex mutex_;
    socket_type downstreamSocket_;
    socket_type upstreamSocket_;
    enum
    {
        max_data_length = 8192
    }; //8KB
    uint8_t downstreamData_[max_data_length];
    uint8_t upstreamData_[max_data_length];

    void HandleUpstreamConnect(const std::error_code& error);
    void HandleUpstreamRead(const std::error_code& error,
        const size_t& bytes_transferred);
    void HandleDownstreamWrite(const std::error_code& error);
    void HandleDownstreamRead(const std::error_code& error,
        const size_t& bytes_transferred);
    void HandleUpstreamWrite(const std::error_code& error);
    void Close();
public:
    explicit Bridge(asio::io_service& ioService) :
        downstreamSocket_(ioService),
        upstreamSocket_(ioService)
    { }
    ~Bridge() = default;

    socket_type& GetDownstreamSocket()
    {
        return downstreamSocket_;
    }
    socket_type& GetUpstreamSocket()
    {
        return upstreamSocket_;
    }
    void Start(const std::string& upstreamHost, uint16_t unstreamPort);
};

