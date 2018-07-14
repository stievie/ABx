#pragma once

#include "Bridge.h"

class Acceptor
{
private:
    asio::io_service& ioService_;
    asio::ip::address_v4 localhostAddress;
    asio::ip::tcp::acceptor acceptor_;
    std::shared_ptr<Bridge> session_;
    unsigned short upstreamPort_;
    std::string upstreamHost_;
    void HandleAccept(const std::error_code& error);
public:
    explicit Acceptor(asio::io_service& ioService,
        const std::string& localHost, uint16_t localPort,
        const std::string& upstreamHost, uint16_t upstreamPort) :
        ioService_(ioService),
        localhostAddress(asio::ip::address_v4::from_string(localHost)),
        acceptor_(ioService_, asio::ip::tcp::endpoint(localhostAddress, localPort)),
        upstreamPort_(upstreamPort),
        upstreamHost_(upstreamHost)
    { }
    ~Acceptor() = default;

    bool AcceptConnections();
};

