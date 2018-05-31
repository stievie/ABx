#pragma once

#include "NetworkMessage.h"
#include <unordered_set>

namespace Net {

class ServicePort;
class OutputMessage;
class Protocol;
class Connection;

class ConnectionManager
{
public:
    static ConnectionManager* Instance()
    {
        static ConnectionManager instance;
        return &instance;
    }
    static uint32_t maxPacketsPerSec;

    std::shared_ptr<Connection> CreateConnection(asio::io_service& ioService, std::shared_ptr<ServicePort> servicer);
    void ReleaseConnection(std::shared_ptr<Connection> connection);
    void CloseAll();
protected:
    ConnectionManager() {}
private:
    std::unordered_set<std::shared_ptr<Connection>> connections_;
    std::mutex lock_;
};

class Connection : public std::enable_shared_from_this<Connection>
{
public:
    enum { WriteTimeout = 30 };
    enum { ReadTimeout = 30 };
    enum State {
        Open = 0,
        Closed = 3
    };
public:
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(asio::io_service& ioService, std::shared_ptr<ServicePort> servicPort) :
        ioService_(ioService),
        socket_(ioService),
        servicePort_(servicPort),
        readTimer_(asio::steady_timer(ioService)),
        writeTimer_(asio::steady_timer(ioService))
    {
        state_ = State::Open;
        receivedFirst_ = false;
        packetsSent_ = 0;
        timeConnected_ = time(nullptr);
    }
    ~Connection();

    /// Send the message
    bool Send(const std::shared_ptr<OutputMessage>& message);
    /// Close the connection
    void Close(bool force = false);
    /// Used by protocols that require server to send first
    void Accept(std::shared_ptr<Protocol> protocol);
    void Accept();
    asio::ip::tcp::socket& GetSocket() { return socket_; }
    uint32_t GetIP() const;
    uint16_t GetPort() const;
protected:
    asio::ip::tcp::socket socket_;
private:
    friend class ConnectionManager;

    static void HandleTimeout(std::weak_ptr<Connection> weakConn, const asio::error_code& error);

    void ParseHeader(const asio::error_code& error);
    void ParsePacket(const asio::error_code& error);
    void CloseSocket();
    void OnWriteOperation(const asio::error_code& error);
    void InternalSend(std::shared_ptr<OutputMessage> message);

    asio::io_service& ioService_;
    std::shared_ptr<ServicePort> servicePort_;
    std::shared_ptr<Protocol> protocol_;
    std::recursive_mutex lock_;
    bool receivedFirst_;
    asio::steady_timer readTimer_;
    asio::steady_timer writeTimer_;
    NetworkMessage msg_;
    std::list<std::shared_ptr<OutputMessage>> messageQueue_;
    time_t timeConnected_;
    uint32_t packetsSent_;

    State state_;
};

}
