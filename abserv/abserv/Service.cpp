#include "stdafx.h"
#include "Service.h"
#include <functional>
#include "Connection.h"
#include "Scheduler.h"
#include <algorithm>

void ServiceManager::Run()
{
    assert(!running_);
    running_ = true;
    ioService_.run();
}

void ServiceManager::Stop()
{
    if (!running_)
        return;

    for (const auto& sp : acceptors_)
    {
        ioService_.post(std::bind(&ServicePort::OnStopServer, sp.second));
    }
    acceptors_.clear();
}

std::list<uint16_t> ServiceManager::GetPorts() const
{
    std::list<uint16_t> ports;
    for (auto it = acceptors_.begin(); it != acceptors_.end(); ++it)
        ports.push_back(it->first);
    // Maps are ordered, so the elements are in order
    ports.unique();
    return ports;
}

void ServiceManager::Die()
{
    ioService_.stop();
}

void ServicePort::Open(uint16_t port)
{
    serverPort_ = port;
    pendingStart_ = false;
    try
    {
        acceptor_ = new asio::ip::tcp::acceptor(service_, asio::ip::tcp::endpoint(
            asio::ip::address(asio::ip::address_v4(INADDR_ANY)), serverPort_));
        Accept();
    }
    catch (asio::system_error& e)
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Network " << e.what() << std::endl;
#else
        UNREFERENCED_PARAMETER(e)
#endif
        // Reschedule
        pendingStart_ = true;
        Scheduler::Instance.Add(CreateScheduledTask(
            5000,
            std::bind(&ServicePort::OpenAcceptor,
                std::weak_ptr<ServicePort>(shared_from_this()),
                serverPort_))
        );
    }
}

void ServicePort::Close()
{
    if (acceptor_)
    {
        if (acceptor_->is_open())
        {
            asio::error_code error;
            acceptor_->close(error);
            if (error)
            {
                LOG_ERROR << "Closing socket " << error.message() << std::endl;
            }
        }
        delete acceptor_;
        acceptor_ = nullptr;
    }
}

bool ServicePort::AddService(std::shared_ptr<ServiceBase> service)
{
    for (std::vector<std::shared_ptr<ServiceBase>>::const_iterator it = services_.begin(); it != services_.end(); ++it)
    {
        if ((*it)->IsSingleSocket())
            return false;
    }

    services_.push_back(service);
    return false;
}

void ServicePort::OnStopServer()
{
    Close();
}

Protocol * ServicePort::MakeProtocol(bool checksummed, NetworkMessage& msg) const
{
    uint8_t protocolId = msg.GetByte();
    for (std::vector<std::shared_ptr<ServiceBase>>::const_iterator it = services_.begin(); it != services_.end(); ++it)
    {
        std::shared_ptr<ServiceBase> service = *it;
        if (service->GetPotocolIdentifier() == protocolId &&
            ((checksummed && service->IsChecksummed()) || !service->IsChecksummed()))
        {
            return service->MakeProtocol(std::shared_ptr<Connection>());
        }
    }
    return nullptr;
}

void ServicePort::Accept()
{
    if (!acceptor_)
        return;

    try
    {
        asio::ip::tcp::socket* socket = new asio::ip::tcp::socket(service_);
        acceptor_->async_accept(*socket,
            std::bind(&ServicePort::OnAccept, this, socket, std::placeholders::_1));
    }
    catch (asio::system_error& e)
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Nettwork " << e.what() << std::endl;
#else
        UNREFERENCED_PARAMETER(e)
#endif
    }
}

void ServicePort::OnAccept(asio::ip::tcp::socket* socket, const asio::error_code& error)
{
    if (!error)
    {
        if (services_.empty())
        {
            return;
        }

        asio::error_code err;
        const asio::ip::tcp::endpoint endpoint = socket->remote_endpoint(err);
        uint32_t remoteIp = 0;
        if (!err)
            remoteIp = htonl(endpoint.address().to_v4().to_ulong());

        if (remoteIp != 0)
        {
            std::shared_ptr<Connection> connection = ConnectionManager::GetInstance()->CreateConnection(
                socket, service_, shared_from_this()
            );

            if (services_.front()->IsSingleSocket())
            {
                // Only one handler, and it will send first
                connection->AcceptConnection(services_.front()->MakeProtocol(connection));
            }
            else
                connection->AcceptConnection();
        }
        else
        {
            // Close connection
            if (socket->is_open())
            {
                asio::error_code err2;
                socket->shutdown(asio::ip::tcp::socket::shutdown_both, err2);
                socket->close(err2);
                delete socket;
            }
        }
#ifdef DEBUG_NET
        LOG_DEBUG << "Accept OK" << std::endl;
#endif
        Accept();
    }
    else if (error != asio::error::operation_aborted)
    {
        Close();
        if (!pendingStart_)
        {
            pendingStart_ = true;
            Scheduler::Instance.Add(CreateScheduledTask(
                5000,
                std::bind(&ServicePort::OpenAcceptor,
                    std::weak_ptr<ServicePort>(shared_from_this()),
                    serverPort_))
            );
        }
    }
#ifdef DEBUG_NET
    else
        LOG_ERROR << "Operation aborted" << std::endl;
#endif
}

void ServicePort::OpenAcceptor(std::weak_ptr<ServicePort> weakService, uint16_t port)
{
    if (auto s = weakService.lock())
    {
        s->Open(port);
    }
}
