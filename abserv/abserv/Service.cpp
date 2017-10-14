#include "stdafx.h"
#include "Service.h"
#include <functional>
#include "Connection.h"

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

void ServiceManager::Die()
{
    ioService_.stop();
}

void ServicePort::Open(uint16_t port)
{
    serverPort_ = port;
    acceptor_ = new asio::ip::tcp::acceptor(service_, asio::ip::tcp::endpoint(
        asio::ip::address(asio::ip::address_v4(INADDR_ANY)), serverPort_));
    Accept();
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

            }
        }
        delete acceptor_;
        acceptor_ = nullptr;
    }
}

bool ServicePort::AddService(std::shared_ptr<ServiceBase> service)
{
    return false;
}

void ServicePort::OnStopServer()
{
    Close();
}

void ServicePort::Accept()
{
    if (!acceptor_)
        return;

    asio::ip::tcp::socket* socket = new asio::ip::tcp::socket(service_);
    acceptor_->async_accept(*socket,
        std::bind(&ServicePort::OnAccept, this, socket, std::placeholders::_1));
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
            }
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
    }
    else if (error == asio::error::operation_aborted)
    {
        Close();
    }

}
