#include "stdafx.h"
#include "ProtocolLogin.h"
#include <AB/ProtocolCodes.h>

#include "DebugNew.h"

namespace Client {

ProtocolLogin::ProtocolLogin() :
    Protocol(),
    charlistCallback(nullptr),
    createAccCallback_(nullptr),
    action_(ActionUnknown)
{
    checksumEnabled_ = ProtocolLogin::UseChecksum;
    encryptEnabled_ = false;
}

void ProtocolLogin::Login(std::string& host, uint16_t port,
    const std::string& account, const std::string& password, const CharlistCallback& callback)
{
    host_ = host;
    port_ = port;
    accountName_ = account;
    password_ = password;
    charlistCallback = callback;
    action_ = ActionLogin;
    Connect(host, port);
}

void ProtocolLogin::CreateAccount(std::string& host, uint16_t port,
    const std::string& account, const std::string& password,
    const std::string& email, const std::string& accKey,
    const CreateAccountCallback& callback)
{
    host_ = host;
    port_ = port;
    accountName_ = account;
    password_ = password;
    email_ = email;
    accKey_ = accKey;
    createAccCallback_ = callback;
    action_ = ActionCreateAccount;
    Connect(host, port);
}

void ProtocolLogin::CreatePlayer(std::string& host, uint16_t port,
    const std::string& account, const std::string& password,
    const std::string& charName, const std::string& prof, PlayerSex sex, bool isPvp,
    const CreatePlayerCallback & callback)
{
    host_ = host;
    port_ = port;
    accountName_ = account;
    password_ = password;
    charName_ = charName;
    prof_ = prof;
    sex_ = sex;
    isPvp_ = isPvp;
    createPlayerCallback_ = callback;
    action_ = ActionCreatePlayer;
    Connect(host, port);
}

void ProtocolLogin::SendLoginPacket()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(ProtocolLogin::ProtocolIdentifier);
    msg->Add<uint16_t>(AB::CLIENT_OS_CURRENT);  // Client OS
    msg->Add<uint16_t>(AB::PROTOCOL_VERSION);   // Protocol Version
    msg->Add<uint8_t>(AB::LoginProtocol::LoginLogin);
    msg->AddStringEncrypted(accountName_);
    msg->AddStringEncrypted(password_);
    Send(msg);
}

void ProtocolLogin::SendCreateAccountPacket()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(ProtocolLogin::ProtocolIdentifier);
    msg->Add<uint16_t>(AB::CLIENT_OS_CURRENT);  // Client OS
    msg->Add<uint16_t>(AB::PROTOCOL_VERSION);   // Protocol Version
    msg->Add<uint8_t>(AB::LoginProtocol::LoginCreateAccount);
    msg->AddStringEncrypted(accountName_);
    msg->AddStringEncrypted(password_);
    msg->AddStringEncrypted(email_);
    msg->AddStringEncrypted(accKey_);
    Send(msg);
}

void ProtocolLogin::SendCreatePlayerPacket()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(ProtocolLogin::ProtocolIdentifier);
    msg->Add<uint16_t>(AB::CLIENT_OS_CURRENT);  // Client OS
    msg->Add<uint16_t>(AB::PROTOCOL_VERSION);   // Protocol Version
    msg->Add<uint8_t>(AB::LoginProtocol::LoginCreateCharacter);
    msg->AddStringEncrypted(accountName_);
    msg->AddStringEncrypted(password_);
    msg->AddStringEncrypted(charName_);
    msg->Add<uint8_t>(static_cast<uint8_t>(sex_));
    msg->AddString(prof_);
    msg->Add<uint8_t>(isPvp_ ? 1 : 0);
    Send(msg);
}

void ProtocolLogin::ParseMessage(const std::shared_ptr<InputMessage>& message)
{
    uint8_t recvByte = message->Get<uint8_t>();
    switch (recvByte)
    {
    case AB::LoginProtocol::CharacterList:
    {
        std::string host = message->GetString();
        if (!host.empty())
            gameHost_ = host;
        gamePort_ = message->Get<uint16_t>();
        CharList chars;
        int count = message->Get<uint8_t>();
        for (int i = 0; i < count; i++)
        {
            uint32_t id = message->Get<uint32_t>();
            uint16_t level = message->Get<uint16_t>();
            std::string name = message->GetString();
            std::string prof = message->GetString();
            std::string prof2 = message->GetString();
            std::string lastMap = message->GetString();
            chars.push_back({ id, level, name, prof, prof2, lastMap });
        }
        if (charlistCallback)
            charlistCallback(chars);
        break;
    }
    case AB::LoginProtocol::LoginError:
    case AB::LoginProtocol::CreateAccountError:
    case AB::LoginProtocol::CreatePlayerError:
    {
        uint8_t error = message->Get<uint8_t>();
        ProtocolError(error);
        break;
    }
    case AB::LoginProtocol::CreateAccountSuccess:
        if (createAccCallback_)
            createAccCallback_();
        break;
    case AB::LoginProtocol::CreatePlayerSuccess:
    {
        std::string playerName = message->GetString();
        std::string map = message->GetString();
        if (createPlayerCallback_)
            createPlayerCallback_(playerName, map);
        break;
    }
    }
}

void ProtocolLogin::OnConnect()
{
    firstRecv_ = true;
    Protocol::OnConnect();

    switch (action_)
    {
    case ActionLogin:
        SendLoginPacket();
        break;
    case ActionCreateAccount:
        SendCreateAccountPacket();
        break;
    case ActionCreatePlayer:
        SendCreatePlayerPacket();
        break;
    default:
        return;
    }
    Receive();
}

void ProtocolLogin::OnReceive(const std::shared_ptr<InputMessage>& message)
{
    if (firstRecv_)
    {
        firstRecv_ = false;
    }

    ParseMessage(message);
}

}
