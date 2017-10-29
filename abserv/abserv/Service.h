#pragma once

#include <memory>
#include <map>
#include <stdint.h>
#include <asio.hpp>
#include "Protocol.h"
#include "Logger.h"

namespace Net {

class Connection;

class ServiceBase
{
public:
    ~ServiceBase() {}
    virtual bool IsSingleSocket() const = 0;
    virtual bool IsChecksummed() const = 0;
    virtual uint8_t GetPotocolIdentifier() const = 0;
    virtual const char* GetProtocolName() const = 0;

    virtual std::shared_ptr<Protocol> MakeProtocol(std::shared_ptr<Connection> c) const = 0;
};

template <typename T>
class Service : public ServiceBase
{
public:
    bool IsSingleSocket() const final
    {
        return T::ServerSendsFirst;
    }
    bool IsChecksummed() const final
    {
        return T::UseChecksum;
    }
    uint8_t GetPotocolIdentifier() const final
    {
        return T::ProtocolIdentifier;
    }
    const char* GetProtocolName() const final
    {
        return T::ProtocolName();
    }

    std::shared_ptr<Protocol> MakeProtocol(std::shared_ptr<Connection> c) const final
    {
        return std::make_shared<T>(c);
    }
};

class ServicePort : public std::enable_shared_from_this<ServicePort>
{
public:
    ServicePort(const ServicePort&) = delete;
    ServicePort& operator=(const ServicePort&) = delete;
    explicit ServicePort(asio::io_service& ioService) :
        service_(ioService),
        serverPort_(0),
        acceptor_(nullptr)
    {}
    ~ServicePort() {}

    void Open(uint16_t port);
    void Close();
    bool AddService(std::shared_ptr<ServiceBase> service);
    bool IsSingleSocket() const
    {
        return services_.size() && services_.front()->IsSingleSocket();
    }
    void OnStopServer();
    std::shared_ptr<Protocol> MakeProtocol(bool checksummed, NetworkMessage& msg, std::shared_ptr<Connection> connection) const;
private:
    using ConstIt = std::vector<std::shared_ptr<ServiceBase>>::const_iterator;
    asio::io_service& service_;
    std::unique_ptr<asio::ip::tcp::acceptor> acceptor_;
    uint16_t serverPort_;
    std::vector<std::shared_ptr<ServiceBase>> services_;
    bool pendingStart_;

    void Accept();
    void OnAccept(std::shared_ptr<Connection> connection, const asio::error_code& error);
    static void OpenAcceptor(std::weak_ptr<ServicePort> weakService, uint16_t port);
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
    bool IsRunning() const { return !acceptors_.empty(); }
    std::list<uint16_t> GetPorts() const;

    /// Adds a protocol and binds it to the port
    template <typename T>
    bool Add(uint16_t port)
    {
        if (port == 0)
        {
#ifdef DEBUG_NET
            LOG_ERROR << "Port can not be 0" << std::endl;
#endif
            return false;
        }
        std::shared_ptr<ServicePort> servicePort;
        std::map<uint16_t, std::shared_ptr<ServicePort>>::iterator finder = acceptors_.find(port);
        if (finder == acceptors_.end())
        {
            servicePort = std::make_shared<ServicePort>(ioService_);
            servicePort->Open(port);
            acceptors_[port] = servicePort;
        }
        else
        {
            servicePort = finder->second;
            if (servicePort->IsSingleSocket() || T::ServerSendsFirst)
            {
                return false;
            }
        }
        return servicePort->AddService(std::shared_ptr<ServiceBase>(new Service<T>()));
    }
private:
    void Die();
    asio::io_service ioService_;
    bool running_;
    std::map<uint16_t, std::shared_ptr<ServicePort>> acceptors_;
};

}
