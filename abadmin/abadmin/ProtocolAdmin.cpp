#include "stdafx.h"
#include "ProtocolAdmin.h"

ProtocolAdmin::ProtocolAdmin()
{
}

ProtocolAdmin::~ProtocolAdmin()
{
}

void ProtocolAdmin::Login(const std::string& host, uint16_t port, const std::string& password)
{
    password_ = password;
    Connect(host, port);
}

void ProtocolAdmin::SendLoginPacket()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(ProtocolAdmin::ProtocolIdentifier);
    Send(msg);
    Receive();
}

void ProtocolAdmin::ParseMessage(const std::shared_ptr<InputMessage>& message)
{
    uint8_t recvByte = message->Get<uint8_t>();
    switch (recvByte)
    {
    case AP_MSG_HELLO:
        ParseMessageHello(message);
        break;
    }
}

void ProtocolAdmin::ParseMessageHello(const std::shared_ptr<InputMessage>& message)
{
    message->Get<uint32_t>();
    serverString_ = message->GetString();
    security_ = message->Get<uint16_t>();
    options_ = message->Get<uint32_t>();
}

void ProtocolAdmin::OnConnect()
{
    firstRecv_ = true;
    Protocol::OnConnect();

    SendLoginPacket();
    Receive();
}

void ProtocolAdmin::OnReceive(const std::shared_ptr<InputMessage>& message)
{

    if (firstRecv_)
    {
        firstRecv_ = false;
    }

    ParseMessage(message);
    Receive();
}

void ProtocolAdmin::OnError(const asio::error_code& err)
{
    Protocol::OnError(err);
}
