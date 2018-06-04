#pragma once

#include "MessageMsg.h"
#include "Logger.h"

namespace Net {

class MessageClient
{
public:
    typedef std::function<void(const asio::error_code& error, size_t bytes_transferred)> ReadHandler;
    typedef std::function<void(const MessageMsg& msg)> MessageHandler;
private:
    asio::io_service& ioService_;
    asio::ip::tcp::socket socket_;
    bool connected_;
    MessageMsg readMsg_;
    MessageQueue writeMsgs_;
    std::string host_;
    uint16_t port_;
    asio::ip::tcp::resolver resolver_;
    MessageHandler messageHandler_;
    void handle_read_header(MessageMsg* msg, const ReadHandler& handler,
        const asio::error_code& error, size_t bytes_transferred)
    {
        if (!error && msg->DecodeHeader())
        {
            asio::async_read(socket_,
                asio::buffer(msg->Body(), msg->BodyLength()),
                handler);
        }
        else
        {
            handler(error, bytes_transferred);
        }
    }
    void async_receive(MessageMsg* msg, const ReadHandler& handler)
    {
        asio::async_read(socket_,
            asio::buffer(msg->Data(), MessageMsg::HeaderLength),
            std::bind(
                &MessageClient::handle_read_header,
                this, msg, handler, std::placeholders::_1, std::placeholders::_2));
    }
    void InternalConnect();
public:
    void HandleRead(const asio::error_code& error, size_t);
    void DoWrite(MessageMsg msg);
    void HandleWrite(const asio::error_code& error);
public:
    MessageClient(asio::io_service& io_service) :
        ioService_(io_service),
        resolver_(io_service),
        socket_(io_service),
        connected_(false)
    {
    };
    ~MessageClient() = default;

    void Connect(const std::string& host, uint16_t port, const MessageHandler& messageHandler);
    void Write(const MessageMsg& msg)
    {
        if (connected_)
            ioService_.post(std::bind(&MessageClient::DoWrite, this, msg));
        else
            LOG_WARNING << "Writing message while not connected" << std::endl;
    }
    void Close()
    {
        socket_.close();
    }
    bool IsConnected() const
    {
        return connected_;
    }
    const std::string& GetHost() const
    {
        return host_;
    }
    uint16_t GetPort() const
    {
        return port_;
    }
};

}
