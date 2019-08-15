#include "stdafx.h"
#include "MessageClient.h"

namespace Net {

void MessageClient::Connect(const std::string& host, uint16_t port, MessageHandler&& messageHandler)
{
    host_ = host;
    port_ = port;
    messageHandler_ = std::move(messageHandler);
    InternalConnect();
}

void MessageClient::InternalConnect()
{
    if (connected_)
        return;

    const asio::ip::tcp::resolver::query query(asio::ip::tcp::v4(), host_, std::to_string(port_));
    asio::ip::tcp::resolver::iterator endpoint = resolver_.resolve(query);
    asio::error_code error;
    socket_.connect(*endpoint, error);
    if (!error)
    {
        connected_ = true;
        AsyncReceive(&readMsg_,
            std::bind(&MessageClient::HandleRead,
                this, std::placeholders::_1, std::placeholders::_2));
    }
    else
        LOG_ERROR << "(" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
}

void MessageClient::HandleRead(const asio::error_code& error, size_t)
{
    if (!connected_)
        return;

    if (!error)
    {
        if (messageHandler_ && !readMsg_.IsEmpty())
            messageHandler_(readMsg_);
        readMsg_.Empty();
        AsyncReceive(&readMsg_,
            std::bind(&MessageClient::HandleRead,
                this, std::placeholders::_1, std::placeholders::_2));
    }
    else
    {
        LOG_ERROR << "(" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
        Close();
    }
}

void MessageClient::DoWrite(MessageMsg msg)
{
    bool write_in_progress = !writeMsgs_.empty();
    writeMsgs_.push_back(msg);
    if (!write_in_progress)
    {
        asio::async_write(socket_,
            asio::buffer(writeMsgs_.front().Data(),
                writeMsgs_.front().Length()),
            std::bind(&MessageClient::HandleWrite, this,
                std::placeholders::_1));
    }
}

void MessageClient::HandleWrite(const asio::error_code& error)
{
    if (!connected_)
        return;

    if (!error)
    {
        writeMsgs_.pop_front();
        if (!writeMsgs_.empty())
        {
            asio::async_write(socket_,
                asio::buffer(writeMsgs_.front().Data(),
                    writeMsgs_.front().Length()),
                std::bind(&MessageClient::HandleWrite, this,
                    std::placeholders::_1));
        }
    }
    else
    {
        LOG_ERROR << "(" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
        Close();
    }
}

}
