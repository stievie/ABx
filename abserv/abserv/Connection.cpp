#include "stdafx.h"
#include "Connection.h"
#include <thread>
#include "Dispatcher.h"
#include "Logger.h"
#include "Protocol.h"
#include <list>
#include "Scheduler.h"
#include "Utils.h"

bool Connection::Send(std::shared_ptr<OutputMessage> message)
{
    return false;
}

void Connection::Close()
{
#ifdef DEBUG_NET
    LOG_DEBUG << "Closing connection" << std::endl;
#endif
    std::lock_guard<std::recursive_mutex> lockClass(lock_);
    if (state_ == State::Closed || state_ == State::RequestClose)
        return;

    state_ = State::RequestClose;

    Dispatcher::Instance.Add(
        CreateTask(std::bind(&Connection::CloseConnectionTask, this))
    );
}

void Connection::AcceptConnection(Protocol* protocol)
{
    protocol_ = protocol;
    protocol_->OnConnect();
    AcceptConnection();
}

void Connection::AcceptConnection()
{
    try
    {
        ++pendingRead_;
        readTimer_.expires_from_now(std::chrono::seconds(Connection::ReadTimeout));
        readTimer_.async_wait(std::bind(&Connection::HandleReadTimeout,
            std::weak_ptr<Connection>(shared_from_this()), std::placeholders::_1));

        // Read size of packet
        asio::async_read(GetHandle(),
            asio::buffer(msg_.GetBuffer(), NetworkMessage::HeaderLength),
            std::bind(&Connection::ParseHeader, shared_from_this(), std::placeholders::_1));
    }
    catch (asio::system_error& e)
    {
        LOG_ERROR << "Network " << e.what() << std::endl;
    }
}

uint32_t Connection::GetIP()
{
    asio::error_code err;
    const asio::ip::tcp::endpoint endpoint = socket_->remote_endpoint(err);
    if (!err)
        return htonl(endpoint.address().to_v4().to_ulong());
    LOG_ERROR << "Getting IP" << std::endl;
    return 0;
}

void Connection::ParseHeader(const asio::error_code& error)
{
    lock_.lock();
    readTimer_.cancel();

    int32_t size = msg_.DecodeHeader();
    if (error || size <= 0 || size >= NETWORKMESSAGE_MAXSIZE - 16)
        HandleReadError(error);

    if (state_ != State::Open || readError_)
    {
        Close();
        lock_.unlock();
        return;
    }

    --pendingRead_;

    try
    {
        readTimer_.expires_from_now(std::chrono::seconds(Connection::ReadTimeout));
        readTimer_.async_wait(std::bind(&Connection::HandleReadTimeout,
            std::weak_ptr<Connection>(shared_from_this()), std::placeholders::_1));

        // Read content
        msg_.SetSize(size + NetworkMessage::HeaderLength);
        asio::async_read(GetHandle(),
            asio::buffer(msg_.GetBodyBuffer(), size),
            std::bind(&Connection::ParsePacket, shared_from_this(), std::placeholders::_1));

    }
    catch (asio::system_error& e)
    {
        LOG_ERROR << "Network " << e.what() << std::endl;
    }

    lock_.unlock();
}

void Connection::ParsePacket(const asio::error_code& error)
{
    lock_.lock();
    readTimer_.cancel();

    if (error)
        HandleReadError(error);

    if (state_ != State::Open || readError_)
    {
        Close();
        lock_.unlock();
        return;
    }

    --pendingRead_;

    uint32_t recvChecksum = msg_.PeekU32();
    uint32_t checksum = 0;
    int32_t len = msg_.GetMessageLength() - msg_.GetReadPos() - 4;
    if (len > 0)
        checksum = AdlerChecksum((uint8_t*)(msg_.GetBuffer() + msg_.GetReadPos() + 4), len);

    if (recvChecksum == checksum)
        // Remove the checksum
        msg_.GetU32();

    if (!receivedFirst_)
    {
        receivedFirst_ = true;
        if (!protocol_)
        {
        }
    }

    lock_.unlock();
}

void Connection::HandleReadError(const asio::error_code& error)
{
}

void Connection::HandleReadTimeout(std::weak_ptr<Connection> weakConn, const asio::error_code& error)
{
}

void Connection::HandleWriteTimeout(std::weak_ptr<Connection> weakConn, const asio::error_code& error)
{
}

void Connection::CloseConnectionTask()
{
    lock_.lock();
    if (state_ != State::RequestClose)
    {
        LOG_ERROR << "state = " << state_ << std::endl;
        lock_.unlock();
        return;
    }

    if (protocol_)
    {
        protocol_->SetConnection(std::shared_ptr<Connection>());
        protocol_->Release();
        protocol_ = nullptr;
    }

    state_ = State::Closing;

    if (pendingWrite_ == 0 || writeError_)
    {
        CloseSocket();
        ReleaseConnection();
        state_ = State::Closed;
    }
    // else will be closed by onWriteOperation/handleWriteTimeout/handleReadTimeout instead

    lock_.unlock();
}

void Connection::CloseSocket()
{
    lock_.lock();

    if (socket_->is_open())
    {
#ifdef DEBUG_NET
        LOG_DEBUG << "Closing socket" << std::endl;
#endif
        pendingRead_ = 0;
        pendingWrite_ = 0;

        try
        {
            asio::error_code err;
            socket_->shutdown(asio::ip::tcp::socket::shutdown_both, err);
            if (err)
            {
                if (err == asio::error::not_connected)
                {
                    // Not connected
                }
                else
                {
                    LOG_ERROR << "Shutdown " << err.value() << ", Message" << err.message() << std::endl;
                }
            }
            socket_->close(err);
            if (err)
            {
                LOG_ERROR << "Close " << err.value() << ", Message" << err.message() << std::endl;
            }
        }
        catch (asio::system_error& e)
        {
            LOG_ERROR << "Network " << e.what() << std::endl;
        }
    }

    lock_.unlock();
}

void Connection::ReleaseConnection()
{
#ifdef DEBUG_NET
    LOG_DEBUG << "Releasing connection" << std::endl;
#endif
    if (refCount_ > 0)
    {
        // Reschedule
        Scheduler::Instance.Add(CreateScheduledTask(SCHEDULER_MINTICKS,
            std::bind(&Connection::ReleaseConnection, this)));
    }
    else
    {
        DeleteConnectionTask();
    }
}

void Connection::DeleteConnectionTask()
{
    assert(refCount_ == 0);
    try
    {
        ioService_.dispatch(std::bind(&Connection::OnStopOperation, this));
    }
    catch (asio::system_error& e)
    {
        LOG_ERROR << "Network " << e.what() << std::endl;
    }
}

void Connection::OnStopOperation()
{
    lock_.lock();

    if (socket_->is_open())
    {
        try
        {
            asio::error_code err;
            socket_->shutdown(asio::ip::tcp::socket::shutdown_both, err);
            socket_->close();
        }
        catch (asio::system_error&)
        { }
    }

    delete socket_;
    socket_ = nullptr;

    lock_.unlock();
    ConnectionManager::GetInstance()->ReleaseConnection(shared_from_this());
}

void Connection::InternalSend(std::shared_ptr<OutputMessage> message)
{
}

std::shared_ptr<Connection> ConnectionManager::CreateConnection(asio::ip::tcp::socket* socket,
    asio::io_service& ioService, std::shared_ptr<ServicePort> servicer)
{
    std::lock_guard<std::recursive_mutex> lock(lock_);
    std::shared_ptr<Connection> connection = std::make_shared<Connection>(socket, ioService, servicer);
    connections_.push_back(connection);

    return connection;
}

void ConnectionManager::ReleaseConnection(std::shared_ptr<Connection> connection)
{
#ifdef DEBUG_NET
    LOG_DEBUG << "Releasing connection" << std::endl;
#endif
    std::lock_guard<std::recursive_mutex> lockClass(lock_);

    auto it = std::find(connections_.begin(), connections_.end(), connection);
    if (it != connections_.end())
        connections_.erase(it);
#ifdef DEBUG_NET
    else
        LOG_ERROR << "Connection not found" << std::endl;
#endif
}

void ConnectionManager::CloseAll()
{
#ifdef DEBUG_NET
    LOG_DEBUG << "Closing all connections" << std::endl;
#endif

    std::lock_guard<std::recursive_mutex> lockClass(lock_);

    for (auto it = connections_.begin(); it != connections_.end();)
    {
        try
        {
            asio::error_code err;
            (*it)->socket_->shutdown(asio::ip::tcp::socket::shutdown_both, err);
            (*it)->socket_->close(err);
        }
        catch (asio::system_error&) {}
    }
    connections_.clear();
}
