#include "stdafx.h"
#include "ProtocolAdmin.h"
#include "Logger.h"

ProtocolAdmin::ProtocolAdmin() :
    Protocol(),
    loggedIn_(false),
    responseCallback_(nullptr)
{
}

ProtocolAdmin::~ProtocolAdmin()
{
}

void ProtocolAdmin::Login(const std::string& host, uint16_t port, const std::string& password)
{
    password_ = password;
    Connect(host, port);
//    Connection::Run();
}

void ProtocolAdmin::SendCommand(char cmdByte, char* command)
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AP_MSG_COMMAND);
    msg->Add<uint8_t>(cmdByte);
    if (command)
        msg->AddString(std::string(command));
    Send(msg);
    Receive();
}

void ProtocolAdmin::SendCommand(char cmdByte, char * command, const ResponseCallback& callback)
{
    responseCallback_ = callback;
    SendCommand(cmdByte, command);
}

void ProtocolAdmin::SendKeepAlive()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(AP_MSG_KEEP_ALIVE);
    Send(msg);
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

#ifdef _LOGGING
    LOG_DEBUG << "recvByte = " << static_cast<unsigned>(recvByte) << std::endl;
#endif
    switch (recvByte)
    {
    case AP_MSG_HELLO:
        HandleMessageHello(message);
        break;
    case AP_MSG_LOGIN_OK:
#ifdef _LOGGING
        LOG_DEBUG << "Login OK" << std::endl;
#endif
        if (responseCallback_)
        {
            responseCallback_(recvByte, message);
            responseCallback_ = nullptr;
        }
        loggedIn_ = true;
        break;
    case AP_MSG_LOGIN_FAILED:
#ifdef _LOGGING
        LOG_DEBUG << "Login failed" << std::endl;
#endif
        if (responseCallback_)
        {
            responseCallback_(recvByte, message);
            responseCallback_ = nullptr;
        }
        break;
    case AP_MSG_COMMAND_OK:
        if (responseCallback_)
        {
            responseCallback_(recvByte, message);
            responseCallback_ = nullptr;
        }
        break;
    case AP_MSG_COMMAND_FAILED:
        if (responseCallback_)
        {
            responseCallback_(recvByte, message);
            responseCallback_ = nullptr;
        }
        break;
    case AP_MSG_ENCRYPTION_OK:
        break;
    case AP_MSG_ENCRYPTION_FAILED:
        break;
    case AP_MSG_PING_OK:
        break;
    case AP_MSG_MESSAGE:
        break;
    case AP_MSG_ERROR:
        break;
    }
}

void ProtocolAdmin::HandleMessageHello(const std::shared_ptr<InputMessage>& message)
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

#ifdef _LOGGING
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
    else
        // No login required
        loggedIn_ = true;
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
//    Receive();
}

void ProtocolAdmin::OnError(const asio::error_code& err)
{
    Protocol::OnError(err);
}
