#pragma once

#include <string>
#include <stdint.h>
#include <functional>
#include <asio.hpp>
#include "Time.h"

namespace Client {

class Connection : public std::enable_shared_from_this<Connection>
{
private:
    typedef std::function<void(const asio::error_code&)> ErrorCallback;
    typedef std::function<void(uint8_t*, uint16_t)> RecvCallback;
    enum {
        ReadTimeout = 30,
        WriteTimeout = 30,
        SendBufferSize = 65536,
        RecvBufferSize = 65536
    };
    void OnResolve(const asio::error_code& error, asio::ip::tcp::resolver::iterator endpointIterator);
    void OnConnectTimeout(const asio::error_code& error);
    void OnReadTimeout(const asio::error_code& error);
    void OnWriteTimeout(const asio::error_code& error);
    void OnConnect(const asio::error_code& error);
    void OnWrite(const asio::error_code& error, size_t writeSize, std::shared_ptr<asio::streambuf> outputStream);
    void OnCanWrite(const asio::error_code& error);
    void OnRecv(const asio::error_code& error, size_t recvSize);
    void HandleError(const asio::error_code& error);
    void InternalConnect(asio::ip::basic_resolver<asio::ip::tcp>::iterator endpointIterator);
    void InternalWrite();
protected:
    std::function<void()> connectCallback_;
    ErrorCallback errorCallback_;
    RecvCallback recvCallback_;
    asio::steady_timer readTimer_;
    asio::steady_timer connectTimer_;
    asio::steady_timer writeTimer_;
    asio::steady_timer delayedWriteTimer_;
    asio::ip::tcp::resolver resolver_;
    asio::ip::tcp::socket socket_;
    bool connected_;
    bool connecting_;
    asio::error_code error_;
    static std::list<std::shared_ptr<asio::streambuf>> outputStreams_;
    std::shared_ptr<asio::streambuf> outputStream_;
    asio::streambuf inputStream_;
    timer activityTimer_;
public:
    Connection();
    ~Connection();

    static void Poll();
    static void Run();
    static void Terminate();

    void Connect(const std::string& host, uint16_t port, const std::function<void()>& connectCallback);
    void Close();

    void Write(uint8_t* buffer, size_t size);
    void Read(uint16_t bytes, const RecvCallback& callback);
    void ReadUntil(const std::string& what, const RecvCallback& callback);
    void ReadSome(const RecvCallback& callback);

    void SetErrorCallback(const ErrorCallback& errorCallback) { errorCallback_ = errorCallback; }

    bool IsConnecting() const { return connecting_; }
    bool IsConnected() const { return connected_; }
};

}
