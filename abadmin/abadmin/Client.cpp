#include "stdafx.h"
#include "Client.h"
#include <iostream>
#include "Rsa.h"
#include "OutputMessage.h"

Client::Client() :
    protocol_(nullptr),
    host_("127.0.0.1"),
    port_(2750)
{
    running_ = true;
    pollThread_ = std::thread([&]() {
        while (running_)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            Connection::Poll();
        }
    });
    keepAliveThread_ = std::thread([&]() {
        while (running_)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            if (IsConnected() && IsLoggedIn())
            {
                protocol_->SendKeepAlive();
            }
        }
    });
}

Client::~Client()
{
    running_ = false;
    keepAliveThread_.join();
    // Let poll() return
    Connection::Terminate();
    pollThread_.join();
}

void Client::Connect(const std::string& pass)
{
    if (!protocol_)
        protocol_ = std::make_shared<ProtocolAdmin>();
    protocol_->Login(host_, port_, pass);
}

bool Client::Disconnect()
{
    if (!IsConnected())
        return false;

    protocol_->Disconnect();
    return true;
}

bool Client::SendCommand(char cmdByte, char* command)
{
    RespStatus s = Pending;
    std::string err;
    protocol_->SendCommand(cmdByte, command, [&](uint8_t recvByte, const std::shared_ptr<InputMessage>& message)
    {
        switch (recvByte)
        {
        case AP_MSG_COMMAND_OK:
            s = Success;
            break;
        case AP_MSG_COMMAND_FAILED:
            err = message->GetString();
            s = Failure;
            break;
        }
    });
    int loops = 0;
    while (s == Pending)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ++loops;
        // Wait max 5sec
        if (loops > 100)
            break;
    }
    if (s == Failure)
        errorMessage_ = err;

    return s == Success;
}
