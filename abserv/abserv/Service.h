#pragma once

#include <memory>
#include <map>
#include <asio.hpp>

class ServicePort : public std::enable_shared_from_this<ServicePort>
{
public:
    ServicePort(const ServicePort&) = delete;
    ServicePort(asio::io_service& ioService) :
        service_(ioService),
        serverPort_(0)
    {}
    ~ServicePort() {}

    void OnStopServer();
private:
    asio::io_service& service_;
    uint16_t serverPort_;
};

class ServiceManager
{
public:
    ServiceManager(const ServiceManager&) = delete;
    ServiceManager() :
        ioService_(),
        running_(false)
    {}
    ~ServiceManager()
    {
        Stop();
    }

    void Run();
    void Stop();
    bool IsRunning() { return !acceptors_.empty(); }
private:
    void Die();
    asio::io_service ioService_;
    bool running_;
    std::map<uint16_t, std::shared_ptr<ServicePort>> acceptors_;
};

