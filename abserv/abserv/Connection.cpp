#include "stdafx.h"
#include "Connection.h"
#include <thread>
#include "Dispatcher.h"
#include "Logger.h"
#include "Protocol.h"

Connection::~Connection()
{
}

bool Connection::Send(std::shared_ptr<OutputMessage> message)
{
    return false;
}

void Connection::Close()
{
    std::lock_guard<std::recursive_mutex> lockClass(connectionLock_);
    if (state_ == State::CONNECTION_STATE_CLOSED || state_ == State::CONNECTION_STATE_REQUEST_CLOSE)
        return;

    state_ = State::CONNECTION_STATE_REQUEST_CLOSE;

    Dispatcher::Instance.Add(
        CreateTask(std::bind(&Connection::CloseConnectionTask, this))
    );
}

void Connection::CloseConnectionTask()
{
    connectionLock_.lock();
    if (state_ != State::CONNECTION_STATE_REQUEST_CLOSE)
    {
        LOG_ERROR << "state = " << state_ << std::endl;
        connectionLock_.unlock();
        return;
    }

    if (protocol_)
    {
        protocol_->SetConnection(std::shared_ptr<Connection>());
        protocol_->Release();
        protocol_ = nullptr;
    }

    state_ = State::CONNECTION_STATE_CLOSING;

    if (pendingWrite_ == 0 || writeError_)
    {
        CloseSocket();
        ReleaseConnection();
        state_ = State::CONNECTION_STATE_CLOSED;
    }
    // else will be closed by onWriteOperation/handleWriteTimeout/handleReadTimeout instead

    connectionLock_.unlock();
}

void Connection::CloseSocket()
{
}

void Connection::ReleaseConnection()
{
}

std::shared_ptr<Connection> ConnectionManager::CreateConnection(asio::ip::tcp::socket* socket,
    asio::io_service& ioService, std::shared_ptr<ServicePort> servicer)
{
    std::lock_guard<std::recursive_mutex> lock(connectionManagerlock_);
    std::shared_ptr<Connection> connection = std::make_shared<Connection>(socket, ioService, servicer);
    connections_.push_back(connection);

    return connection;
}

void ConnectionManager::ReleaseConnection(std::shared_ptr<Connection> connection)
{
}

void ConnectionManager::CloseAll()
{
}
