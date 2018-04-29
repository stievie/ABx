#include "stdafx.h"
#include "ProtocolLogin.h"
#include <AB/ProtocolCodes.h>
#include <AB/Entities/Character.h>

#include "DebugNew.h"

namespace Client {

ProtocolLogin::ProtocolLogin() :
    Protocol(),
    charlistCallback_(nullptr),
    gamelistCallback_(nullptr),
    createAccCallback_(nullptr),
    action_(ActionUnknown)
{
    checksumEnabled_ = ProtocolLogin::UseChecksum;
    encryptEnabled_ = false;
}

void ProtocolLogin::Login(std::string& host, uint16_t port,
    const std::string& account, const std::string& password,
    const CharlistCallback& callback)
{
    host_ = host;
    port_ = port;
    accountName_ = account;
    password_ = password;
    charlistCallback_ = callback;
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
    const std::string& charName, const std::string& prof, AB::Entities::CharacterSex sex, bool isPvp,
    const CreatePlayerCallback& callback)
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

void ProtocolLogin::GetGameList(std::string& host, uint16_t port,
    const std::string& account, const std::string& password,
    const GamelistCallback& callback)
{
    host_ = host;
    port_ = port;
    accountName_ = account;
    password_ = password;
    gamelistCallback_ = callback;
    action_ = ActionGetGameList;
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

void ProtocolLogin::SendGetGameListPacket()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(ProtocolLogin::ProtocolIdentifier);
    msg->Add<uint16_t>(AB::CLIENT_OS_CURRENT);  // Client OS
    msg->Add<uint16_t>(AB::PROTOCOL_VERSION);   // Protocol Version
    msg->Add<uint8_t>(AB::LoginProtocol::LoginGetGameList);
    msg->AddStringEncrypted(accountName_);
    msg->AddStringEncrypted(password_);
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
        /* int charSlots = */ message->Get<uint16_t>();
        AB::Entities::CharacterList chars;
        int count = message->Get<uint16_t>();
        for (int i = 0; i < count; ++i)
        {
            AB::Entities::Character cData;
            cData.uuid = message->GetStringEncrypted();
            cData.level = message->Get<uint8_t>();
            cData.name = message->GetStringEncrypted();
            cData.profession = message->GetStringEncrypted();
            cData.profession2 = message->GetStringEncrypted();
            cData.sex = static_cast<AB::Entities::CharacterSex>(message->Get<uint8_t>());
            cData.lastMap = message->GetStringEncrypted();
            chars.push_back(cData);
        }
        if (charlistCallback_)
            charlistCallback_(chars);
        break;
    }
    case AB::LoginProtocol::GameList:
    {
        std::vector<AB::Entities::Game> games;
        int count = message->Get<uint16_t>();
        for (int i = 0; i < count; ++i)
        {
            AB::Entities::Game g;
            g.uuid = message->GetStringEncrypted();
            g.name = message->GetStringEncrypted();
            g.type = static_cast<AB::Entities::GameType>(message->Get<uint8_t>());
            games.push_back(g);
        }
        if (gamelistCallback_)
            gamelistCallback_(games);
        break;
    }
    case AB::LoginProtocol::LoginError:
    case AB::LoginProtocol::CreateAccountError:
    case AB::LoginProtocol::CreatePlayerError:
    case AB::LoginProtocol::AddAccountKeyError:
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
        std::string playerName = message->GetStringEncrypted();
        std::string map = message->GetStringEncrypted();
        if (createPlayerCallback_)
            createPlayerCallback_(playerName, map);
        break;
    }
    case AB::LoginProtocol::AddAccountKeySuccess:
        // TODO:
        break;
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
    case ActionGetGameList:
        SendGetGameListPacket();
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
