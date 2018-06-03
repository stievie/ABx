#pragma once

#include "MessageMsg.h"
#include "Logger.h"

namespace Net {

class MessageClient
{
private:
    typedef asio::ip::tcp stream_protocol;
    typedef typename stream_protocol::socket socket;
public:
    typedef std::function<void(const asio::error_code& error, size_t bytes_transferred)> ReadHandler;
private:
    asio::io_service& ioService_;
    socket socket_;
    bool connected_;
    MessageMsg readMsg_;
    MessageQueue writeMsgs_;
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
public:
    void HandleConnect(const asio::error_code& error);
    void HandleRead(const asio::error_code& error, size_t);
    void DoWrite(MessageMsg msg);
    void HandleWrite(const asio::error_code& error);
public:
    MessageClient(asio::io_service& io_service, asio::ip::tcp::resolver::iterator endpoint_iterator) :
        ioService_(io_service),
        socket_(io_service),
        connected_(false)
    {
        asio::async_connect(socket_, endpoint_iterator,
            std::bind(&MessageClient::HandleConnect, this,
                std::placeholders::_1));
    };
    ~MessageClient() = default;
    void Write(const MessageMsg& msg)
    {
        ioService_.post(std::bind(&MessageClient::DoWrite, this, msg));
    }
    void Close()
    {
        socket_.close();
    }
};

}
