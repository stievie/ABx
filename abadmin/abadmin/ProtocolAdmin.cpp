#include "stdafx.h"
#include "ProtocolAdmin.h"
#include "Logger.h"

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
    Connection::Run();
}

void ProtocolAdmin::SendLoginPacket()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(ProtocolAdmin::ProtocolIdentifier);
    Send(msg);
    Receive();
    Connection::Run();
}

void ProtocolAdmin::ParseMessage(const std::shared_ptr<InputMessage>& message)
{
    uint8_t recvByte = message->Get<uint8_t>();

#ifdef _DEBUG
    LOG_DEBUG << "recvByte = " << static_cast<unsigned>(recvByte) << std::endl;
#endif
    switch (recvByte)
    {
    case AP_MSG_HELLO:
        ParseMessageHello(message);
        break;
    case AP_MSG_LOGIN_OK:
#ifdef _DEBUG
        LOG_DEBUG << "Login OK" << std::endl;
#endif
        break;
    case AP_MSG_LOGIN_FAILED:
#ifdef _DEBUG
        LOG_DEBUG << "Login failed" << std::endl;
#endif
        break;
    }
}

void ProtocolAdmin::ParseMessageHello(const std::shared_ptr<InputMessage>& message)
{
    message->Get<uint32_t>();
    serverString_ = message->GetString();
    security_ = message->Get<uint16_t>();
    options_ = message->Get<uint32_t>();
    if (security_ & REQUIRE_ENCRYPTION)
    {
        serverString_ += " encryption";
        if (options_ & ENCRYPTION_RSA1024XTEA)
            serverString_ += "(RSA1024XTEA)";
        else
            serverString_ += "(Not supported)";
    }
    if (security_ & REQUIRE_LOGIN)
        serverString_ += " login";

#ifdef _DEBUG
    LOG_DEBUG << "Hello from " << serverString_ << std::endl;
#endif

    if (security_ & REQUIRE_ENCRYPTION)
    {
        SetupEncryption();
    }

    if (security_ & REQUIRE_LOGIN)
    {
        DoLogin();
    }
}

void ProtocolAdmin::DoLogin()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AP_MSG_LOGIN);
    msg->AddString(password_);
    Send(msg);
    Receive();
    Connection::Run();
}

void ProtocolAdmin::SetupEncryption()
{
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
