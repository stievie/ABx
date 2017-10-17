#pragma once

#include <memory>
#include <asio.hpp>
#include <list>
#include <mutex>
#include "RefCounted.h"
#include "NetworkMessage.h"

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
    std::recursive_mutex lock_;
};

class Connection : public std::enable_shared_from_this<Connection>, public RefCounted
{
public:
    enum { WriteTimeout = 30 };
    enum { ReadTimeout = 30 };
    enum State {
        Open = 0,
        RequestClose = 1,
        Closing = 2,
        Closed = 3
    };
public:
    Connection(const Connection&) = delete;
    Connection(asio::ip::tcp::socket* socket,
        asio::io_service& ioService, std::shared_ptr<ServicePort> servicPort) :
        socket_(socket),
        ioService_(ioService),
        servicePort_(servicPort),
        state_(State::Open),
        protocol_(nullptr),
        readTimer_(asio::steady_timer(ioService)),
        writeTimer_(asio::steady_timer(ioService))
    {}
    ~Connection() {}

    /// Send the message
    bool Send(std::shared_ptr<OutputMessage> message);
    /// Close the connection
    void Close();
    void AcceptConnection(Protocol* protocol);
    void AcceptConnection();
    asio::ip::tcp::socket& GetHandle() { return *socket_; }
    uint32_t GetIP();
protected:
    asio::ip::tcp::socket* socket_;
private:
    friend class ConnectionManager;

    static void HandleReadTimeout(std::weak_ptr<Connection> weakConn, const asio::error_code& error);
    static void HandleWriteTimeout(std::weak_ptr<Connection> weakConn, const asio::error_code& error);

    void ParseHeader(const asio::error_code& error);
    void ParsePacket(const asio::error_code& error);
    void HandleReadError(const asio::error_code& error);
    void CloseConnectionTask();
    void CloseSocket();
    void ReleaseConnection();
    void DeleteConnectionTask();
    void OnStopOperation();
    void InternalSend(std::shared_ptr<OutputMessage> message);
    asio::io_service& ioService_;
    std::shared_ptr<ServicePort> servicePort_;
    std::recursive_mutex lock_;
    Protocol* protocol_;
    int32_t pendingWrite_;
    int32_t pendingRead_;
    bool writeError_;
    bool readError_;
    bool receivedFirst_;
    asio::steady_timer readTimer_;
    asio::steady_timer writeTimer_;
    NetworkMessage msg_;

    State state_;
};

