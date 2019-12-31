#include "stdafx.h"
#include "ProtocolLogin.h"
#include <AB/ProtocolCodes.h>
#include <AB/Entities/Character.h>
#include <AB/Packets/Packet.h>

namespace Client {

ProtocolLogin::ProtocolLogin(Crypto::DHKeys& keys, asio::io_service& ioService) :
    Protocol(keys, ioService),
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
    const std::string& accountUuid, const std::string& token,
    const std::string& charName, const std::string& profUuid,
    uint32_t itemIndex,
    AB::Entities::CharacterSex sex, bool isPvp,
    const CreatePlayerCallback& callback)
{
    host_ = host;
    port_ = port;
    accountUuid_ = accountUuid;
    authToken_ = token;
    charName_ = charName;
    profUuid_ = profUuid;
    itemIndex_ = itemIndex;
    sex_ = sex;
    isPvp_ = isPvp;
    createPlayerCallback_ = callback;
    action_ = ActionCreatePlayer;
    Connect(host, port);
}

void ProtocolLogin::GetOutposts(std::string& host, uint16_t port,
    const std::string& accountUuid, const std::string& token,
    const GamelistCallback& callback)
{
    host_ = host;
    port_ = port;
    accountUuid_ = accountUuid;
    authToken_ = token;
    gamelistCallback_ = callback;
    action_ = ActionGetOutposts;
    Connect(host, port);
}

void ProtocolLogin::GetServers(std::string& host, uint16_t port,
    const std::string& accountUuid, const std::string& token,
    const ServerlistCallback& callback)
{
    host_ = host;
    port_ = port;
    accountUuid_ = accountUuid;
    authToken_ = token;
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
    AB::Packets::Client::Login::Login packet = {
        accountName_,
        password_
    };
    AB::Packets::Add(packet, *msg);
    Send(std::move(msg));
}

void ProtocolLogin::SendCreateAccountPacket()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(ProtocolLogin::ProtocolIdentifier);
    msg->Add<uint16_t>(AB::CLIENT_OS_CURRENT);  // Client OS
    msg->Add<uint16_t>(AB::PROTOCOL_VERSION);   // Protocol Version
    msg->Add<uint8_t>(AB::LoginProtocol::LoginCreateAccount);
    AB::Packets::Client::Login::CreateAccount packet = {
        accountName_,
        password_,
        email_,
        accKey_
    };
    AB::Packets::Add(packet, *msg);
    Send(std::move(msg));
}

void ProtocolLogin::SendCreatePlayerPacket()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(ProtocolLogin::ProtocolIdentifier);
    msg->Add<uint16_t>(AB::CLIENT_OS_CURRENT);  // Client OS
    msg->Add<uint16_t>(AB::PROTOCOL_VERSION);   // Protocol Version
    msg->Add<uint8_t>(AB::LoginProtocol::LoginCreateCharacter);
    AB::Packets::Client::Login::CreatePlayer packet = {
        accountUuid_,
        authToken_,
        charName_,
        itemIndex_,
        sex_,
        profUuid_,
        isPvp_
    };
    AB::Packets::Add(packet, *msg);
    Send(std::move(msg));
}

void ProtocolLogin::SendGetOutpostsPacket()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(ProtocolLogin::ProtocolIdentifier);
    msg->Add<uint16_t>(AB::CLIENT_OS_CURRENT);  // Client OS
    msg->Add<uint16_t>(AB::PROTOCOL_VERSION);   // Protocol Version
    msg->Add<uint8_t>(AB::LoginProtocol::LoginGetOutposts);
    AB::Packets::Client::Login::GetOutposts packet = {
        accountUuid_,
        authToken_
    };
    AB::Packets::Add(packet, *msg);
    Send(std::move(msg));
}

void ProtocolLogin::SendGetServersPacket()
{
    std::shared_ptr<OutputMessage> msg = OutputMessage::New();
    msg->Add<uint8_t>(ProtocolLogin::ProtocolIdentifier);
    msg->Add<uint16_t>(AB::CLIENT_OS_CURRENT);  // Client OS
    msg->Add<uint16_t>(AB::PROTOCOL_VERSION);   // Protocol Version
    msg->Add<uint8_t>(AB::LoginProtocol::LoginGetGameServers);
    AB::Packets::Client::Login::GetServers packet = {
        accountUuid_,
        authToken_
    };
    AB::Packets::Add(packet, *msg);
    Send(std::move(msg));
}

void ProtocolLogin::HandleCharList(const AB::Packets::Server::Login::CharacterList& packet)
{
    if (!packet.serverHost.empty())
        gameHost_ = packet.serverHost;
    gamePort_ = packet.serverPort;
    if (!packet.fileHost.empty())
        fileHost_ = packet.fileHost;
    filePort_ = packet.filePort;
    loggedInCallback_(packet.accountUuid, packet.authToken);

    AB::Entities::CharList chars;
    for (const auto& c : packet.characters)
    {
        AB::Entities::Character cData;
        cData.uuid = c.uuid;
        cData.level = c.level;
        cData.name = c.name;
        cData.profession = c.profession;
        cData.profession2 = c.profession2;
        cData.sex = static_cast<AB::Entities::CharacterSex>(c.sex);
        cData.modelIndex = c.modelIndex;
        cData.lastOutpostUuid = c.outpostUuid;
        chars.push_back(cData);
    }
    if (charlistCallback_)
        charlistCallback_(chars);
}

void ProtocolLogin::HandleOutpostList(const AB::Packets::Server::Login::OutpostList& packet)
{
    std::vector<AB::Entities::Game> games;
    for (const auto& o : packet.outposts)
    {
        AB::Entities::Game g;
        g.uuid = o.uuid;
        g.name = o.name;
        g.type = static_cast<AB::Entities::GameType>(o.type);
        g.partySize = o.partySize;
        g.mapCoordX = o.coordX;
        g.mapCoordY = o.coordY;
        games.push_back(g);
    }
    if (gamelistCallback_)
        gamelistCallback_(games);
}

void ProtocolLogin::HandleServerList(const AB::Packets::Server::Login::ServerList& packet)
{
    std::vector<AB::Entities::Service> servers;
    for (const auto& _s : packet.servers)
    {
        AB::Entities::Service s;
        s.type = static_cast<AB::Entities::ServiceType>(_s.type);
        s.uuid = _s.uuid;
        s.host = _s.host;
        s.port = _s.port;
        s.location = _s.location;
        s.name = _s.name;
        servers.push_back(s);
    }
    if (serverlistCallback_)
        serverlistCallback_(servers);
}

void ProtocolLogin::HandleLoginError(const AB::Packets::Server::Login::Error& packet)
{
    ProtocolError(packet.code);
}

void ProtocolLogin::HandleCreatePlayerSuccess(const AB::Packets::Server::Login::CreateCharacterSuccess& packet)
{
    if (createPlayerCallback_)
        createPlayerCallback_(packet.uuid, packet.mapUuid);
}

void ProtocolLogin::ParseMessage(InputMessage& message)
{
    uint8_t recvByte = message.Get<uint8_t>();
    switch (recvByte)
    {
    case AB::LoginProtocol::CharacterList:
    {
        auto packet = AB::Packets::Get<AB::Packets::Server::Login::CharacterList>(message);
        HandleCharList(packet);
        break;
    }
    case AB::LoginProtocol::OutpostList:
    {
        auto packet = AB::Packets::Get<AB::Packets::Server::Login::OutpostList>(message);
        HandleOutpostList(packet);
        break;
    }
    case AB::LoginProtocol::ServerList:
    {
        auto packet = AB::Packets::Get<AB::Packets::Server::Login::ServerList>(message);
        HandleServerList(packet);
        break;
    }
    case AB::LoginProtocol::LoginError:
    case AB::LoginProtocol::CreateAccountError:
    case AB::LoginProtocol::CreatePlayerError:
    case AB::LoginProtocol::AddAccountKeyError:
    case AB::LoginProtocol::DeletePlayerError:
    {
        auto packet = AB::Packets::Get<AB::Packets::Server::Login::Error>(message);
        HandleLoginError(packet);
        break;
    }
    case AB::LoginProtocol::CreateAccountSuccess:
        if (createAccCallback_)
            createAccCallback_();
        break;
    case AB::LoginProtocol::CreatePlayerSuccess:
    {
        auto packet = AB::Packets::Get<AB::Packets::Server::Login::CreateCharacterSuccess>(message);
        HandleCreatePlayerSuccess(packet);
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

void ProtocolLogin::OnReceive(InputMessage& message)
{
    if (firstRecv_)
        firstRecv_ = false;

    ParseMessage(message);
}

}
