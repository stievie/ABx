#include "stdafx.h"
#include "ProtocolLogin.h"
#include <AB/ProtocolCodes.h>
#include <AB/Entities/Character.h>

#include "DebugNew.h"

namespace Client {

ProtocolLogin::ProtocolLogin(Crypto::DHKeys& keys) :
    Protocol(keys),
    action_(ActionUnknown),
    charlistCallback_(nullptr),
    gamelistCallback_(nullptr),
    createAccCallback_(nullptr)
{
    checksumEnabled_ = ProtocolLogin::UseChecksum;
    encryptEnabled_ = false;
    SetEncKey(AB::ENC_KEY);
}

void ProtocolLogin::Login(std::string& host, uint16_t port,
    const std::string& account, const std::string& password,
    const LoggedInCallback& onLoggedIn,
    const CharlistCallback& callback)
{
    host_ = host;
    port_ = port;
    accountName_ = account;
    password_ = password;
    loggedInCallback_ = onLoggedIn;
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
    const std::string& accountUuid, const std::string& password,
    const std::string& charName, const std::string& profUuid,
    uint32_t modelIndex,
    AB::Entities::CharacterSex sex, bool isPvp,
    const CreatePlayerCallback& callback)
{
    host_ = host;
    port_ = port;
    accountUuid_ = accountUuid;
    password_ = password;
    charName_ = charName;
    profUuid_ = profUuid;
    modelIndex_ = modelIndex;
    sex_ = sex;
    isPvp_ = isPvp;
    createPlayerCallback_ = callback;
    action_ = ActionCreatePlayer;
    Connect(host, port);
}

void ProtocolLogin::GetOutposts(std::string& host, uint16_t port,
    const std::string& accountUuid, const std::string& password,
    const GamelistCallback& callback)
{
    host_ = host;
    port_ = port;
    accountUuid_ = accountUuid;
    password_ = password;
    gamelistCallback_ = callback;
    action_ = ActionGetOutposts;
    Connect(host, port);
}

void ProtocolLogin::GetServers(std::string& host, uint16_t port,
    const std::string& accountUuid, const std::string& password,
    const ServerlistCallback& callback)
{
    host_ = host;
    port_ = port;
    accountUuid_ = accountUuid;
    password_ = password;
    serverlistCallback_ = callback;
    action_ = ActionGetServers;
    Connect(host, port);
}

void ProtocolLogin::SendLoginPacket()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
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
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
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
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(ProtocolLogin::ProtocolIdentifier);
    msg->Add<uint16_t>(AB::CLIENT_OS_CURRENT);  // Client OS
    msg->Add<uint16_t>(AB::PROTOCOL_VERSION);   // Protocol Version
    msg->Add<uint8_t>(AB::LoginProtocol::LoginCreateCharacter);
    msg->AddStringEncrypted(accountUuid_);
    msg->AddStringEncrypted(password_);
    msg->AddStringEncrypted(charName_);
    msg->Add<uint32_t>(modelIndex_);
    msg->Add<uint8_t>(static_cast<uint8_t>(sex_));
    msg->AddString(profUuid_);
    msg->Add<uint8_t>(isPvp_ ? 1 : 0);
    Send(msg);
}

void ProtocolLogin::SendGetOutpostsPacket()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(ProtocolLogin::ProtocolIdentifier);
    msg->Add<uint16_t>(AB::CLIENT_OS_CURRENT);  // Client OS
    msg->Add<uint16_t>(AB::PROTOCOL_VERSION);   // Protocol Version
    msg->Add<uint8_t>(AB::LoginProtocol::LoginGetOutposts);
    msg->AddStringEncrypted(accountUuid_);
    msg->AddStringEncrypted(password_);
    Send(msg);
}

void ProtocolLogin::SendGetServersPacket()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(ProtocolLogin::ProtocolIdentifier);
    msg->Add<uint16_t>(AB::CLIENT_OS_CURRENT);  // Client OS
    msg->Add<uint16_t>(AB::PROTOCOL_VERSION);   // Protocol Version
    msg->Add<uint8_t>(AB::LoginProtocol::LoginGetGameServers);
    msg->AddStringEncrypted(accountUuid_);
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
        std::string accountUuid = message->GetStringEncrypted();
        std::string host = message->GetString();
        if (!host.empty())
            gameHost_ = host;
        gamePort_ = message->Get<uint16_t>();
        host = message->GetString();
        if (!host.empty())
            fileHost_ = host;
        filePort_ = message->Get<uint16_t>();
        loggedInCallback_(accountUuid);

        /* int charSlots = */ message->Get<uint16_t>();
        AB::Entities::CharList chars;
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
            cData.modelIndex = message->Get<uint32_t>();
            cData.lastOutpostUuid = message->GetStringEncrypted();
            chars.push_back(cData);
        }
        if (charlistCallback_)
            charlistCallback_(chars);
        break;
    }
    case AB::LoginProtocol::OutpostList:
    {
        std::vector<AB::Entities::Game> games;
        int count = message->Get<uint16_t>();
        for (int i = 0; i < count; ++i)
        {
            AB::Entities::Game g;
            g.uuid = message->GetStringEncrypted();
            g.name = message->GetStringEncrypted();
            g.type = static_cast<AB::Entities::GameType>(message->Get<uint8_t>());
            g.partySize = message->Get<uint8_t>();
            g.mapCoordX = message->Get<int32_t>();
            g.mapCoordY = message->Get<int32_t>();
            games.push_back(g);
        }
        if (gamelistCallback_)
            gamelistCallback_(games);
        break;
    }
    case AB::LoginProtocol::ServerList:
    {
        std::vector<AB::Entities::Service> servers;
        int count = message->Get<uint16_t>();
        for (int i = 0; i < count; i++)
        {
            AB::Entities::Service s;
            s.type = message->Get<AB::Entities::ServiceType>();
            s.uuid = message->GetStringEncrypted();
            s.host = message->GetStringEncrypted();
            s.port = message->Get<uint16_t>();
            s.location = message->GetStringEncrypted();
            s.name = message->GetStringEncrypted();
            servers.push_back(s);
        }
        if (serverlistCallback_)
            serverlistCallback_(servers);
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
        std::string playerUuid = message->GetStringEncrypted();
        std::string mapUuid = message->GetStringEncrypted();
        if (createPlayerCallback_)
            createPlayerCallback_(playerUuid, mapUuid);
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
    case ActionGetOutposts:
        SendGetOutpostsPacket();
        break;
    case ActionGetServers:
        SendGetServersPacket();
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
