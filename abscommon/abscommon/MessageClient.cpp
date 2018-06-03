#include "stdafx.h"
#include "MessageClient.h"

namespace Net {

void MessageClient::HandleConnect(const asio::error_code& error)
{
    if (!error)
    {
        connected_ = true;
        this->async_receive(&readMsg_,
            std::bind(&MessageClient::HandleRead,
                this, std::placeholders::_1, std::placeholders::_2));
    }
    else
    {
        LOG_ERROR << "(" << error.value() << ") " << error.message() << std::endl;
    }
}
void MessageClient::HandleRead(const asio::error_code& error, size_t)
{
    if (!error)
    {
        //            std::cout << ">> ";
        //            std::cout.write(readMsg_.Body(), readMsg_.BodyLength());
        //            std::cout << "\n";
        this->async_receive(&readMsg_,
            std::bind(&MessageClient::HandleRead,
                this, std::placeholders::_1, std::placeholders::_2));
    }
    else
    {
        LOG_ERROR << "(" << error.value() << ") " << error.message() << std::endl;
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
        LOG_ERROR << "(" << error.value() << ") " << error.message() << std::endl;
        Close();
    }
}

}