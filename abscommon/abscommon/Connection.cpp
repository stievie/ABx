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

#include "stdafx.h"
#include "Connection.h"
#include "Dispatcher.h"
#include "Logger.h"
#include "Protocol.h"
#include "Scheduler.h"
#include "StringUtils.h"
#include "Service.h"
#include "OutputMessage.h"
#include "Subsystems.h"

namespace Net {

uint32_t ConnectionManager::maxPacketsPerSec = 0;

Connection::Connection(asio::io_service& ioService, std::shared_ptr<ServicePort> servicPort) :
    socket_(ioService),
    servicePort_(servicPort),
    readTimer_(asio::steady_timer(ioService)),
    writeTimer_(asio::steady_timer(ioService)),
    msg_(NetworkMessage::GetNew())
{
    state_ = State::Open;
    receivedFirst_ = false;
    packetsSent_ = 0;
    timeConnected_ = time(nullptr);
}

Connection::~Connection()
{
    CloseSocket();
}

bool Connection::Send(sa::SharedPtr<OutputMessage>&& message)
{

    if (state_ != State::Open)
        return false;

    OutputMessage& msg = *message;
    bool noPendingWrite = messageQueue_.empty();
    std::lock_guard<std::mutex> lockClass(lock_);
    messageQueue_.emplace_back(std::move(message));
    if (noPendingWrite)
        InternalSend(msg);

    return true;
}

void Connection::InternalSend(OutputMessage& message)
{
    protocol_->OnSendMessage(message);
    try
    {
        writeTimer_.expires_from_now(std::chrono::seconds(Connection::WriteTimeout));
        writeTimer_.async_wait(std::bind(&Connection::HandleWriteTimeout,
            std::weak_ptr<Connection>(shared_from_this()), std::placeholders::_1));

        asio::async_write(socket_,
            asio::buffer(message.GetOutputBuffer(), static_cast<size_t>(message.GetSize())),
            std::bind(&Connection::OnWriteOperation, shared_from_this(), std::placeholders::_1));
    }
    catch (asio::system_error& e)
    {
        LOG_ERROR << "Network " << e.code() << " " << e.what() << std::endl;
    }
}

void Connection::OnWriteOperation(const asio::error_code& error)
{
    std::lock_guard<std::mutex> lockClass(lock_);
    writeTimer_.cancel();
    messageQueue_.pop_front();

    if (error)
    {
        messageQueue_.clear();
        Close(true);
        return;
    }

    if (!messageQueue_.empty())
        InternalSend(*messageQueue_.front());
    else if (state_ == State::Closed)
        CloseSocket();
}

void Connection::Close(bool force /* = false */)
{
    auto* connMngr = GetSubsystem<ConnectionManager>();
    if (!connMngr)
    {
        LOG_ERROR << "No ConnectionManager subsystem." << std::endl;
        return;
    }
    connMngr->ReleaseConnection(shared_from_this());

    if (state_ != State::Open)
        return;
    state_ = State::Closed;
    if (protocol_)
    {
        GetSubsystem<Asynch::Dispatcher>()->Add(
            Asynch::CreateTask(std::bind(&Protocol::Release, protocol_))
        );
    }

    if (messageQueue_.empty() || force)
        CloseSocket();
}

void Connection::Accept(std::shared_ptr<Protocol> protocol)
{
    protocol_ = protocol;
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(&Protocol::OnConnect, protocol))
    );
    Accept();
}

void Connection::Accept()
{
    try
    {
        readTimer_.expires_from_now(std::chrono::seconds(Connection::ReadTimeout));
        readTimer_.async_wait(std::bind(&Connection::HandleReadTimeout,
            std::weak_ptr<Connection>(shared_from_this()), std::placeholders::_1));

        // Read size of packet
        asio::async_read(socket_,
            asio::buffer(msg_->GetBuffer(), NetworkMessage::HeaderLength),
            std::bind(&Connection::ParseHeader, shared_from_this(), std::placeholders::_1));
    }
    catch (asio::system_error& e)
    {
        LOG_ERROR << "Network " << e.code() << " " << e.what() << std::endl;
        Close(true);
    }
}

void Connection::ParseHeader(const asio::error_code& error)
{
    readTimer_.cancel();
#ifdef DEBUG_NET
    lastReadHeader_ = Utils::Tick();
#endif

    if (error)
    {
#ifdef DEBUG_NET
        // Maybe disconnect
//        if (error.value() != 995)
            LOG_ERROR << "Network " << error.value() << " " << error.message() << std::endl;
#endif
        Close(true);
        return;
    }
    if (state_ != State::Open)
    {
#ifdef DEBUG_NET
        LOG_WARNING << "Connection not opened" << std::endl;
#endif
        return;
    }

    uint32_t timePassed = std::max<uint32_t>(1, static_cast<uint32_t>(time(nullptr) - timeConnected_) + 1);
    ++packetsSent_;
    if ((ConnectionManager::maxPacketsPerSec != 0) && (packetsSent_ / timePassed) > ConnectionManager::maxPacketsPerSec)
    {
        LOG_WARNING << Utils::ConvertIPToString(GetIP()) << " disconnected for exceeding packet per second limit." << std::endl;
        Close(true);
        return;
    }

    if (timePassed > 2)
    {
        timeConnected_ = time(nullptr);
        packetsSent_ = 0;
    }

    int32_t size = msg_->GetHeaderSize();
    if (size == 0 || size >= static_cast<int32_t>(NetworkMessage::NETWORKMESSAGE_BUFFER_SIZE) - 16)
    {
        LOG_WARNING << "Invalid message size " << size << " disconnecting client" << std::endl;
        Close(true);
        return;
    }

    try
    {
        readTimer_.expires_from_now(std::chrono::seconds(Connection::ReadTimeout));
        readTimer_.async_wait(std::bind(&Connection::HandleReadTimeout,
            std::weak_ptr<Connection>(shared_from_this()), std::placeholders::_1));

        // Read content
        msg_->SetSize(size + NetworkMessage::HeaderLength);
        asio::async_read(socket_,
            asio::buffer(msg_->GetBodyBuffer(), static_cast<size_t>(size)),
            std::bind(&Connection::ParsePacket, shared_from_this(), std::placeholders::_1));

    }
    catch (asio::system_error& e)
    {
        LOG_ERROR << "Network " << e.code() << " " << e.what() << std::endl;
        Close(true);
    }
}

void Connection::ParsePacket(const asio::error_code& error)
{
    readTimer_.cancel();
#ifdef DEBUG_NET
    lastReadBody_ = Utils::Tick();
#endif

    if (error)
    {
        Close(true);
        return;
    }
    if (state_ != State::Open)
    {
#ifdef DEBUG_NET
        LOG_WARNING << "Connection not opened" << std::endl;
#endif
        return;
    }

    uint32_t checksum;
    int32_t len = msg_->GetSize() - msg_->GetReadPos() - NetworkMessage::ChecksumLength;
    if (len > 0)
        checksum = Utils::AdlerChecksum(
            reinterpret_cast<uint8_t*>(msg_->GetBuffer() + msg_->GetReadPos() +
                NetworkMessage::ChecksumLength),
            len);
    else
        checksum = 0;
    uint32_t recvChecksum = msg_->Get<uint32_t>();
    if (recvChecksum != checksum)
        // it might not have been the checksum, step back
        msg_->Skip(-NetworkMessage::ChecksumLength);

    if (!receivedFirst_)
    {
        // First message received
        receivedFirst_ = true;

        if (!protocol_)
        {
            // Game protocol has already been created at this point
            protocol_ = servicePort_->MakeProtocol(recvChecksum == checksum, *msg_, shared_from_this());
            if (!protocol_)
            {
                Close(true);
                return;
            }

        }
        else
            // Skip protocol ID
            msg_->Skip(1);

        protocol_->OnRecvFirstMessage(*msg_);
    }
    else
        // Send the packet to the current protocol
        protocol_->OnRecvMessage(*msg_);

    try
    {
        readTimer_.expires_from_now(std::chrono::seconds(Connection::ReadTimeout));
        readTimer_.async_wait(std::bind(&Connection::HandleReadTimeout,
            std::weak_ptr<Connection>(shared_from_this()), std::placeholders::_1));

        // Wait for the next packet
        asio::async_read(socket_,
            asio::buffer(msg_->GetBuffer(), NetworkMessage::HeaderLength),
            std::bind(&Connection::ParseHeader, shared_from_this(), std::placeholders::_1));
    }
    catch (asio::system_error& e)
    {
        LOG_ERROR << "Network " << e.code() << " " << e.what() << std::endl;
        Close(true);
    }
}

void Connection::HandleReadTimeout(std::weak_ptr<Connection> weakConn, const asio::error_code& error)
{
    if (error == asio::error::operation_aborted)
        return;

    // It needs a constant stream of packets or the connection will be closed.
    // Send at least a ping once in a while.

    if (auto conn = weakConn.lock())
    {
#ifdef DEBUG_NET
        int64_t now = Utils::Tick();
        LOG_DEBUG << "Read Timeout, closing connection. Error(" << error.value() << ") " << error.message()
            << ", lastreadheader " << (now - conn->lastReadHeader_)
            << ", lastreadbody " << (now - conn->lastReadBody_)
            << std::endl;
#endif
        conn->Close(true);
    }
}

void Connection::HandleWriteTimeout(std::weak_ptr<Connection> weakConn, const asio::error_code& error)
{
    if (error == asio::error::operation_aborted)
        return;

    // It needs a constant stream of packets or the connection will be closed.
    // Send at least a ping once in a while.
#ifdef DEBUG_NET
    LOG_DEBUG << "Write Timeout, closing connection. Error(" << error.value() << ") " << error.message() << std::endl;
#endif

    if (auto conn = weakConn.lock())
        conn->Close(true);
}

void Connection::CloseSocket()
{
    if (socket_.is_open())
    {
        readTimer_.cancel();
        writeTimer_.cancel();
        asio::error_code err;
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both, err);
        if (err)
            LOG_ERROR << "Network " << err.default_error_condition().value() << " " << err.default_error_condition().message() << std::endl;

        asio::error_code err2;
        socket_.close(err2);
        if (err2)
            LOG_ERROR << "Network " << err2.default_error_condition().value() << " " << err2.default_error_condition().message() << std::endl;
    }
}

std::shared_ptr<Connection> ConnectionManager::CreateConnection(
    asio::io_service& ioService, std::shared_ptr<ServicePort> servicer)
{
    if (connections_.size() >= SERVER_MAX_CONNECTIONS)
    {
        LOG_ERROR << "Too many connections" << std::endl;
        return std::shared_ptr<Connection>();
    }

    std::shared_ptr<Connection> connection = std::make_shared<Connection>(ioService, servicer);
    {
        std::lock_guard<std::mutex> lock(lock_);
        connections_.insert(connection);
    }

    return connection;
}

void ConnectionManager::ReleaseConnection(std::shared_ptr<Connection> connection)
{
    std::lock_guard<std::mutex> lockClass(lock_);
    connections_.erase(connection);
}

void ConnectionManager::CloseAll()
{
    std::lock_guard<std::mutex> lockClass(lock_);
    for (const auto& conn : connections_)
        conn->CloseSocket();
    connections_.clear();
}

uint32_t Connection::GetIP() const
{
    asio::error_code err;
    const asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint(err);
    if (!err)
        return endpoint.address().to_v4().to_uint();
    return 0;
}

uint16_t Connection::GetPort() const
{
    asio::error_code err;
    const asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint(err);
    if (!err)
        return endpoint.port();
    return 0;
}

}
