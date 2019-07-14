#include "stdafx.h"
#include "Connection.h"
#include <AB/CommonConfig.h>

// https://www.gamedev.net/blogs/entry/2249317-a-guide-to-getting-started-with-boostasio/

namespace Client {

std::list<std::shared_ptr<asio::streambuf>> Connection::outputStreams_;

Connection::Connection(asio::io_service& ioService) :
    readTimer_(ioService),
    connectTimer_(ioService),
    writeTimer_(ioService),
    delayedWriteTimer_(ioService),
    resolver_(ioService),
    socket_(ioService),
    connected_(false),
    connecting_(false)
{ }

Connection::~Connection()
{
    Close();
}

void Connection::Terminate()
{
    outputStreams_.clear();
}

void Connection::Connect(const std::string& host, uint16_t port, const std::function<void()>& connectCallback)
{
    connected_ = false;
    connecting_ = true;
    error_.clear();
    connectCallback_ = connectCallback;

    connectTimer_.cancel();
    connectTimer_.expires_from_now(std::chrono::seconds(Connection::ConnectTimeout));
    connectTimer_.async_wait(std::bind(&Connection::OnConnectTimeout, shared_from_this(),
        std::placeholders::_1));

    asio::ip::tcp::resolver::query query(asio::ip::tcp::v4(), host, std::to_string(port));
    resolver_.async_resolve(query, std::bind(&Connection::OnResolve, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2));
}

void Connection::OnResolve(const asio::error_code& error, asio::ip::tcp::resolver::iterator endpointIterator)
{
    connectTimer_.cancel();
    if (error == asio::error::operation_aborted)
        return;

    if (!error)
        InternalConnect(endpointIterator);
    else
        HandleError(ConnectionError::ResolveError, error);
}

void Connection::OnConnectTimeout(const asio::error_code& error)
{
    if (error == asio::error::operation_aborted)
        return;
    HandleError(ConnectionError::ConnectTimeout, error);
}

void Connection::OnReadTimeout(const asio::error_code& error)
{
    if (error == asio::error::operation_aborted)
        return;
    HandleError(ConnectionError::ReadTimeout, error);
}

void Connection::OnWriteTimeout(const asio::error_code& error)
{
    if (error == asio::error::operation_aborted)
        return;
    HandleError(ConnectionError::WriteTimeout, error);
}

void Connection::OnConnect(const asio::error_code& error)
{
    connectTimer_.cancel();
    activityTimer_.restart();

    if (error == asio::error::operation_aborted)
        return;

    if (!error)
    {
        connected_ = true;
#ifdef TCP_OPTION_NODELAY
        // disable nagle's algorithm, this make the game play smoother
        socket_.set_option(asio::ip::tcp::no_delay(true));
#endif
        if (connectCallback_)
            connectCallback_();
    }
    else
        HandleError(ConnectionError::ConnectError, error);

    connecting_ = false;
}

void Connection::OnWrite(const asio::error_code& error, size_t writeSize,
    std::shared_ptr<asio::streambuf> outputStream)
{
    AB_UNUSED(writeSize);

    writeTimer_.cancel();

    if (error == asio::error::operation_aborted)
        return;

    // free output stream and store for using it again later
    outputStream->consume(outputStream->size());
    outputStreams_.push_back(outputStream);

    if (connected_ && error)
        HandleError(ConnectionError::WriteError, error);
}

void Connection::HandleError(ConnectionError connectionError, const asio::error_code& error)
{
    if (error == asio::error::operation_aborted)
        return;

    error_ = error;
    if (errorCallback_)
        errorCallback_(connectionError, error);

    if (connected_ || connecting_)
        Close();
}

void Connection::InternalConnect(asio::ip::basic_resolver<asio::ip::tcp>::iterator endpointIterator)
{
    connectTimer_.cancel();
    connectTimer_.expires_from_now(std::chrono::seconds(Connection::ConnectTimeout));
    connectTimer_.async_wait(std::bind(&Connection::OnConnectTimeout, shared_from_this(), std::placeholders::_1));

    socket_.async_connect(*endpointIterator,
        std::bind(&Connection::OnConnect, shared_from_this(), std::placeholders::_1));
}

void Connection::InternalWrite()
{
    if (!connected_)
        return;

    std::shared_ptr<asio::streambuf> outputStream = outputStream_;
    outputStream_.reset();

    asio::async_write(socket_,
        *outputStream,
        std::bind(&Connection::OnWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2, outputStream));

    writeTimer_.cancel();
    writeTimer_.expires_from_now(std::chrono::seconds(Connection::WriteTimeout));
    writeTimer_.async_wait(std::bind(&Connection::OnWriteTimeout, shared_from_this(), std::placeholders::_1));
}

void Connection::Close()
{
    if (!connected_ && !connecting_)
        return;

    // Flush send data before disconnecting on clean connections
    if (connected_ && !error_ && outputStream_)
        InternalWrite();

    connecting_ = false;
    connected_ = false;
    connectCallback_ = nullptr;
    errorCallback_ = nullptr;
    recvCallback_ = nullptr;

    resolver_.cancel();
    connectTimer_.cancel();
    readTimer_.cancel();
    writeTimer_.cancel();
    delayedWriteTimer_.cancel();

    if (socket_.is_open())
    {
        asio::error_code err;
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both, err);
        socket_.close();
    }
}

void Connection::Write(uint8_t* buffer, size_t size)
{
    if (!connected_)
        return;

    // We can't send the data right away, otherwise we could create tcp congestion
    if (!outputStream_)
    {
        if (!outputStreams_.empty())
        {
            outputStream_ = outputStreams_.front();
            outputStreams_.pop_front();
        }
        else
            outputStream_ = std::make_shared<asio::streambuf>();

        delayedWriteTimer_.cancel();
        delayedWriteTimer_.expires_from_now(std::chrono::milliseconds(10));
        delayedWriteTimer_.async_wait(std::bind(&Connection::OnCanWrite, shared_from_this(), std::placeholders::_1));
    }

    std::ostream os(outputStream_.get());
    os.write((const char*)buffer, size);
    os.flush();
}

void Connection::OnCanWrite(const asio::error_code& error)
{
    delayedWriteTimer_.cancel();

    if (error == asio::error::operation_aborted)
        return;

    if (connected_)
        InternalWrite();
}

void Connection::OnRecv(const asio::error_code& error, size_t recvSize)
{
    readTimer_.cancel();
    activityTimer_.restart();

    if (error == asio::error::operation_aborted)
        return;

    if (connected_)
    {
        if (!error)
        {
            if (recvCallback_)
            {
                const char* header = asio::buffer_cast<const char*>(inputStream_.data());
                recvCallback_((uint8_t*)header, (uint16_t)recvSize);
            }
        }
        else
            HandleError(ConnectionError::ReceiveError, error);
    }

    if (!error)
        inputStream_.consume(recvSize);
}

void Connection::Read(uint16_t bytes, const RecvCallback& callback)
{
    if (!connected_)
        return;

    recvCallback_ = callback;

    readTimer_.cancel();
    readTimer_.expires_from_now(std::chrono::seconds(Connection::ReadTimeout));
    readTimer_.async_wait(std::bind(&Connection::OnReadTimeout, shared_from_this(), std::placeholders::_1));

    asio::async_read(socket_,
        asio::buffer(inputStream_.prepare(bytes)),
        std::bind(&Connection::OnRecv, shared_from_this(),
            std::placeholders::_1, std::placeholders::_2));

}

void Connection::ReadUntil(const std::string& what, const RecvCallback& callback)
{
    if (!connected_)
        return;

    recvCallback_ = callback;

    readTimer_.cancel();
    readTimer_.expires_from_now(std::chrono::seconds(Connection::ReadTimeout));
    readTimer_.async_wait(std::bind(&Connection::OnReadTimeout, shared_from_this(), std::placeholders::_1));

    asio::async_read_until(socket_,
        inputStream_,
        what.c_str(),
        std::bind(&Connection::OnRecv, shared_from_this(), std::placeholders::_1, std::placeholders::_2));

}

void Connection::ReadSome(const RecvCallback& callback)
{
    if (!connected_)
        return;

    recvCallback_ = callback;

    readTimer_.cancel();
    readTimer_.expires_from_now(std::chrono::seconds(Connection::ReadTimeout));
    readTimer_.async_wait(std::bind(&Connection::OnReadTimeout, shared_from_this(), std::placeholders::_1));

    socket_.async_read_some(asio::buffer(inputStream_.prepare(RecvBufferSize)),
        std::bind(&Connection::OnRecv, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

}
