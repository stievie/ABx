#pragma once

#include "Bridge.h"
#include <AB/Entities/Service.h>

class Acceptor
{
public:
    typedef std::function<bool(AB::Entities::Service& svc)> GetServiceCallback;
private:
    asio::io_service& ioService_;
    asio::ip::address_v4 localhostAddress;
    asio::ip::tcp::acceptor acceptor_;
    std::shared_ptr<Bridge> session_;
    GetServiceCallback getServiceCallback_;
    void HandleAccept(const std::error_code& error);
public:
    explicit Acceptor(asio::io_service& ioService,
        const std::string& localHost, uint16_t localPort,
        const GetServiceCallback& getServiceCallback) :
        ioService_(ioService),
        localhostAddress(asio::ip::address_v4::from_string(localHost)),
        acceptor_(ioService_, asio::ip::tcp::endpoint(localhostAddress, localPort)),
        getServiceCallback_(getServiceCallback)
    { }
    ~Acceptor() = default;

    bool AcceptConnections();
};

