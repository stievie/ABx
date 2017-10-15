#pragma once

#include <memory>
#include <asio.hpp>
#include <list>
#include <mutex>
#include "RefCounted.h"

class Connection;
class ServicePort;
class OutputMessage;
class Protocol;

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

class Connection : public std::enable_shared_from_this<Connection>, public RefCounted
{
public:
    enum State {
        CONNECTION_STATE_OPEN = 0,
        CONNECTION_STATE_REQUEST_CLOSE = 1,
        CONNECTION_STATE_CLOSING = 2,
        CONNECTION_STATE_CLOSED = 3
    };
public:
    Connection(const Connection&) = delete;
    Connection(asio::ip::tcp::socket* socket,
        asio::io_service& ioService, std::shared_ptr<ServicePort> servicPort) :
        socket_(socket),
        ioService_(ioService),
        servicePort_(servicPort),
        state_(CONNECTION_STATE_OPEN),
        protocol_(nullptr)
    {}
    ~Connection();

    /// Send the message
    bool Send(std::shared_ptr<OutputMessage> message);
    /// Close the connection
    void Close();
private:
    void CloseConnectionTask();
    void CloseSocket();
    void ReleaseConnection();
    asio::ip::tcp::socket* socket_;
    asio::io_service& ioService_;
    std::shared_ptr<ServicePort> servicePort_;
    std::recursive_mutex connectionLock_;
    Protocol* protocol_;
    int32_t pendingWrite_;
    int32_t pendingRead_;
    bool writeError_;
    bool readError_;

    State state_;
};

