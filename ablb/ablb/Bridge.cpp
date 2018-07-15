#include "stdafx.h"
#include "Bridge.h"

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
