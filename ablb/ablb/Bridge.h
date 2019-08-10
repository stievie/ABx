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

