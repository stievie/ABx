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

#include "Protocol.h"
#include "Logger.h"
#include <map>
#include <list>
#include <functional>
#include <asio.hpp>
#include <sa/Noncopyable.h>

namespace Net {

class Connection;

class ServiceBase
{
public:
    virtual ~ServiceBase() = default;
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

using AcceptConnection = std::function<bool(uint32_t remoteIp)>;

class ServicePort : public std::enable_shared_from_this<ServicePort>
{
    NON_COPYABLE(ServicePort)
public:
    explicit ServicePort(asio::io_service& ioService, AcceptConnection&& acceptConnection) :
        service_(ioService),
        serverPort_(0),
        serverIp_(INADDR_ANY),
        acceptor_(nullptr),
        acceptConnection_(std::move(acceptConnection))
    {}
    ~ServicePort() = default;

    bool Open(uint32_t ip, uint16_t port);
    void Close();
    bool AddService(std::shared_ptr<ServiceBase> service);
    bool IsSingleSocket() const
    {
        return services_.size() && services_.front()->IsSingleSocket();
    }
    void OnStopServer();
    std::shared_ptr<Protocol> MakeProtocol(bool checksummed, NetworkMessage& msg,
        std::shared_ptr<Connection> connection) const;
private:
    using ConstIt = std::vector<std::shared_ptr<ServiceBase>>::const_iterator;
    asio::io_service& service_;
    uint16_t serverPort_;
    uint32_t serverIp_;
    std::unique_ptr<asio::ip::tcp::acceptor> acceptor_;
    AcceptConnection acceptConnection_;

    std::vector<std::shared_ptr<ServiceBase>> services_;

    void Accept();
    void OnAccept(std::shared_ptr<Connection> connection, const asio::error_code& error);
};

class ServiceManager
{
    NON_COPYABLE(ServiceManager)
private:
    using AcceptorKey = std::pair<uint32_t, uint16_t>;
public:
    ServiceManager(asio::io_service& ioService) :
        ioService_(ioService),
        running_(false)
    {}
    ~ServiceManager()
    {
        Stop();
    }

    void Run();
    void Stop();
    bool IsRunning() const { return !acceptors_.empty(); }
    std::list<std::pair<uint32_t, uint16_t>> GetPorts() const;

    /// Adds a protocol and binds it to the port
    template <typename T>
    bool Add(uint32_t ip, uint16_t port, AcceptConnection&& acceptConnection)
    {
        if (port == 0)
        {
#ifdef DEBUG_NET
            LOG_ERROR << "Port can not be 0" << std::endl;
#endif
            return false;
        }
        std::shared_ptr<ServicePort> servicePort;
        AcceptorKey key(ip, port);
        const auto finder = acceptors_.find(key);
        if (finder == acceptors_.end())
        {
            servicePort = std::make_shared<ServicePort>(ioService_, std::move(acceptConnection));
            if (!servicePort->Open(ip, port))
                return false;
            acceptors_[key] = servicePort;
        }
        else
        {
            servicePort = finder->second;
            if (servicePort->IsSingleSocket() || T::ServerSendsFirst)
                return false;
        }
        return servicePort->AddService(std::make_shared<Service<T>>());
    }
    /// Get a free random port
    static uint16_t GetFreePort();
private:
    asio::io_service& ioService_;
    bool running_;
    std::map<AcceptorKey, std::shared_ptr<ServicePort>> acceptors_;
};

}
