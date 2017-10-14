#pragma once

#include <memory>
#include <asio.hpp>
#include <list>
#include <mutex>

class Connection;
class ServicePort;
class OutputMessage;

class ConnectionManager
{
public:
    ~ConnectionManager() {}
    static ConnectionManager* GetInstance()
    {
        static ConnectionManager instance;
        return &instance;
    }

    std::shared_ptr<Connection> CreateConnection(asio::ip::tcp::socket* socket,
        asio::io_service& ioService, std::shared_ptr<ServicePort> servicer);
    void ReleaseConnection(std::shared_ptr<Connection> connection);
    void CloseAll();
protected:
    ConnectionManager() {}
private:
    std::list<std::shared_ptr<Connection>> connections_;
    std::recursive_mutex connectionManagerlock_;
};

class Connection : public std::enable_shared_from_this<Connection>
{
public:
    Connection(const Connection&) = delete;
    Connection(asio::ip::tcp::socket* socket,
        asio::io_service& ioService, std::shared_ptr<ServicePort> servicPort) :
        socket_(socket),
        ioService_(ioService),
        servicePort_(servicPort)
    {}
    ~Connection();

    bool Send(std::shared_ptr<OutputMessage> message);
private:
    asio::ip::tcp::socket* socket_;
    asio::io_service& ioService_;
    std::shared_ptr<ServicePort> servicePort_;
};

