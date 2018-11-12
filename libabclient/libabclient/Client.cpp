#include "stdafx.h"
#include "Client.h"
#include "ProtocolLogin.h"
#include "ProtocolGame.h"
#include "Connection.h"
#define USE_STANDALONE_ASIO
#pragma warning(push)
#pragma warning(disable: 4457 4456 4150)
#include <SimpleWeb/client_https.hpp>
#pragma warning(pop)
#include <iostream>
#include <fstream>

#include "DebugNew.h"

namespace Client {

class HttpsClient : public SimpleWeb::Client<SimpleWeb::HTTPS>
{
public:
    HttpsClient(const std::string &server_port_path, bool verify_certificate = true,
        const std::string &cert_file = std::string(),
        const std::string &private_key_file = std::string(),
        const std::string &verify_file = std::string()) :
        SimpleWeb::Client<SimpleWeb::HTTPS>::Client(server_port_path, verify_certificate,
            cert_file, private_key_file, verify_file)
    { }
};

Client::Client() :
    loginHost_("127.0.0.1"),
    gameHost_(""),
    fileHost_(""),
    loginPort_(2748),
    gamePort_(0),
    filePort_(0),
    protoLogin_(nullptr),
    protoGame_(nullptr),
    state_(ClientState::Disconnected),
    lastRun_(0),
    lastPing_(0),
    gotPong_(true),
    httpClient_(nullptr)
{
    // Always create new keys
    arc4random_stir();
    dhKeys_.GenerateKeys();
}

Client::~Client()
{
    if (httpClient_)
        delete httpClient_;
    Connection::Terminate();
}

void Client::Poll()
{
    Connection::Poll();
}

void Client::Run()
{
    Connection::Run();
}

void Client::OnLoggedIn(const std::string& accountUuid)
{
    accountUuid_ = accountUuid;
    gamePort_ = protoLogin_->gamePort_;
    if (!protoLogin_->gameHost_.empty())
        gameHost_ = protoLogin_->gameHost_;
    else
        // If game host is empty use the login host
        gameHost_ = loginHost_;

    filePort_ = protoLogin_->filePort_;
    if (!protoLogin_->fileHost_.empty())
        fileHost_ = protoLogin_->fileHost_;
    else
        // If file host is empty use the login host
        fileHost_ = loginHost_;

    if (!fileHost_.empty() && filePort_ != 0)
    {
        std::stringstream ss;
        ss << fileHost_ << ":" << filePort_;
        httpClient_ = new HttpsClient(ss.str(), false);
    }
    if (receiver_)
        receiver_->OnLoggedIn(accountUuid_);
}

void Client::OnGetCharlist(const AB::Entities::CharList& chars)
{
    state_ = ClientState::SelectChar;
    if (receiver_)
        receiver_->OnGetCharlist(chars);

    // Get list of outposts
    GetOutposts();
}

void Client::OnGetOutposts(const std::vector<AB::Entities::Game>& games)
{
    if (receiver_)
        receiver_->OnGetOutposts(games);
}

void Client::OnGetServices(const std::vector<AB::Entities::Service>& services)
{
    if (receiver_)
        receiver_->OnGetServices(services);
}

void Client::OnGetMailHeaders(int64_t updateTick, const std::vector<AB::Entities::MailHeader>& headers)
{
    if (receiver_)
        receiver_->OnGetMailHeaders(updateTick, headers);
}

void Client::OnGetMail(int64_t updateTick, const AB::Entities::Mail& mail)
{
    if (receiver_)
        receiver_->OnGetMail(updateTick, mail);
}

void Client::OnEnterWorld(int64_t updateTick, const std::string& serverId,
    const std::string& mapUuid, uint32_t playerId)
{
    state_ = ClientState::World;
    mapUuid_ = mapUuid;
    if (receiver_)
        receiver_->OnEnterWorld(updateTick, serverId, mapUuid, playerId);
}

void Client::OnChangeInstance(int64_t updateTick, const std::string& serverId,
    const std::string& mapUuid, const std::string& instanceUuid, const std::string& charUuid)
{
    if (receiver_)
        receiver_->OnChangeInstance(updateTick, serverId, mapUuid, instanceUuid, charUuid);
}

void Client::OnDespawnObject(int64_t updateTick, uint32_t id)
{
    if (receiver_)
        receiver_->OnDespawnObject(updateTick, id);
}

void Client::OnObjectPos(int64_t updateTick, uint32_t id, const Vec3& pos)
{
    if (receiver_)
        receiver_->OnObjectPos(updateTick, id, pos);
}

void Client::OnObjectRot(int64_t updateTick, uint32_t id, float rot, bool manual)
{
    if (receiver_)
        receiver_->OnObjectRot(updateTick, id, rot, manual);
}

void Client::OnObjectStateChange(int64_t updateTick, uint32_t id, AB::GameProtocol::CreatureState state)
{
    if (receiver_)
        receiver_->OnObjectStateChange(updateTick, id, state);
}

void Client::OnObjectSpeedChange(int64_t updateTick, uint32_t id, float speedFactor)
{
    if (receiver_)
        receiver_->OnObjectSpeedChange(updateTick, id, speedFactor);
}

void Client::OnAccountCreated()
{
    if (receiver_)
        receiver_->OnAccountCreated();
}

void Client::OnPlayerCreated(const std::string& uuid, const std::string& mapUuid)
{
    if (receiver_)
        receiver_->OnPlayerCreated(uuid, mapUuid);
}

void Client::OnObjectSelected(int64_t updateTick, uint32_t sourceId, uint32_t targetId)
{
    if (receiver_)
        receiver_->OnObjectSelected(updateTick, sourceId, targetId);
}

void Client::OnServerMessage(int64_t updateTick, AB::GameProtocol::ServerMessageType type,
    const std::string& senderName, const std::string& message)
{
    if (receiver_)
        receiver_->OnServerMessage(updateTick, type, senderName, message);
}

void Client::OnChatMessage(int64_t updateTick, AB::GameProtocol::ChatMessageChannel channel,
    uint32_t senderId, const std::string& senderName, const std::string& message)
{
    if (receiver_)
        receiver_->OnChatMessage(updateTick, channel, senderId, senderName, message);
}

void Client::OnPartyInvited(int64_t updateTick, uint32_t sourceId, uint32_t targetId)
{
    if (receiver_)
        receiver_->OnPartyInvited(updateTick, sourceId, targetId);
}

void Client::OnPartyRemoved(int64_t updateTick, uint32_t sourceId, uint32_t targetId)
{
    if (receiver_)
        receiver_->OnPartyRemoved(updateTick, sourceId, targetId);
}

void Client::OnPartyAdded(int64_t updateTick, uint32_t sourceId, uint32_t targetId)
{
    if (receiver_)
        receiver_->OnPartyAdded(updateTick, sourceId, targetId);
}

void Client::OnPartyInviteRemoved(int64_t updateTick, uint32_t sourceId, uint32_t targetId)
{
    if (receiver_)
        receiver_->OnPartyInviteRemoved(updateTick, sourceId, targetId);
}

void Client::OnSpawnObject(int64_t updateTick, uint32_t id, const ObjectSpawn& objectSpawn,
    PropReadStream& data, bool existing)
{
    if (receiver_)
        receiver_->OnSpawnObject(updateTick, id, objectSpawn, data, existing);
}

void Client::OnNetworkError(const std::error_code& err)
{
    if (state_ != ClientState::Disconnected && receiver_)
        receiver_->OnNetworkError(err);
}

void Client::OnProtocolError(uint8_t err)
{
    if (receiver_)
        receiver_->OnProtocolError(err);
}

void Client::OnPong(int lastPing)
{
    gotPong_ = true;
    while (pings_.size() > 9)
        pings_.erase(pings_.begin());
    pings_.push_back(lastPing);
}

void Client::OnServerJoined(const AB::Entities::Service& service)
{
    if (receiver_)
        receiver_->OnServerJoined(service);
}

void Client::OnServerLeft(const AB::Entities::Service& service)
{
    if (receiver_)
        receiver_->OnServerLeft(service);
}

std::shared_ptr<ProtocolLogin> Client::GetProtoLogin()
{
    if (!protoLogin_)
    {
        protoLogin_ = std::make_shared<ProtocolLogin>(dhKeys_);
        protoLogin_->SetErrorCallback(std::bind(&Client::OnNetworkError, this, std::placeholders::_1));
        protoLogin_->SetProtocolErrorCallback(std::bind(&Client::OnProtocolError, this, std::placeholders::_1));
    }
    return protoLogin_;
}

void Client::Login(const std::string& name, const std::string& pass)
{
    if (!(state_ == ClientState::Disconnected || state_ == ClientState::CreateAccount))
        return;

    accountName_ = name;
    password_ = pass;

    // 1. Login to login server -> get character list
    GetProtoLogin()->Login(loginHost_, loginPort_, name, pass,
        std::bind(&Client::OnLoggedIn, this, std::placeholders::_1),
        std::bind(&Client::OnGetCharlist, this, std::placeholders::_1));
    Connection::Run();
}

void Client::CreateAccount(const std::string& name, const std::string& pass,
    const std::string& email, const std::string& accKey)
{
    if (state_ != ClientState::CreateAccount)
        return;

    accountName_ = name;
    password_ = pass;

    GetProtoLogin()->CreateAccount(loginHost_, loginPort_, name, pass,
        email, accKey,
        std::bind(&Client::OnAccountCreated, this));
    Connection::Run();
}

void Client::CreatePlayer(const std::string& charName, const std::string& profUuid,
    uint32_t modelIndex,
    AB::Entities::CharacterSex sex, bool isPvp)
{
    if (state_ != ClientState::SelectChar)
        return;

    if (accountUuid_.empty() || password_.empty())
        return;

    GetProtoLogin()->CreatePlayer(loginHost_, loginPort_, accountUuid_, password_,
        charName, profUuid, modelIndex, sex, isPvp,
        std::bind(&Client::OnPlayerCreated, this, std::placeholders::_1, std::placeholders::_2));
    Connection::Run();
}

void Client::Logout()
{
    if (state_ != ClientState::World)
        return;
    if (protoGame_)
    {
        state_ = ClientState::Disconnected;
        protoGame_->Logout();
        Connection::Run();
    }
}

void Client::GetOutposts()
{
    if (accountUuid_.empty() || password_.empty())
        return;

    GetProtoLogin()->GetOutposts(loginHost_, loginPort_, accountUuid_, password_,
        std::bind(&Client::OnGetOutposts, this, std::placeholders::_1));
    Connection::Run();
}

void Client::GetServers()
{
    if (accountUuid_.empty() || password_.empty())
        return;

    GetProtoLogin()->GetServers(loginHost_, loginPort_, accountUuid_, password_,
        std::bind(&Client::OnGetServices, this, std::placeholders::_1));
    Connection::Run();
}

void Client::EnterWorld(const std::string& charUuid, const std::string& mapUuid,
    const std::string& host /* = "" */, uint16_t port /* = 0 */, const std::string& instanceId /* = "" */)
{
    assert(!accountUuid_.empty());
    // Enter or changing the world
    if (state_ != ClientState::SelectChar && state_ != ClientState::World)
        return;

    if (state_ == ClientState::World)
    {
        // We are already logged in to some world so we must logout
        Logout();
        gotPong_ = true;
    }

    // Maybe different server
    if (!host.empty())
        gameHost_ = host;
    if (port != 0)
        gamePort_ = port;

    // 2. Login to game server
    if (!protoGame_)
    {
        protoGame_ = std::make_shared<ProtocolGame>(dhKeys_);
        protoGame_->receiver_ = this;
    }

    protoGame_->Login(accountUuid_, password_, charUuid, mapUuid, instanceId,
        gameHost_, gamePort_);
}

void Client::Update(int timeElapsed)
{
    if (state_ == ClientState::World)
    {
        if (lastPing_ >= 1000 && gotPong_)
        {
            if (protoGame_)
            {
                gotPong_ = false;
                protoGame_->Ping();
            }
            lastPing_ = 0;
        }
    }

    lastRun_ += timeElapsed;
    if (lastRun_ >= 15)
    {
        // Don't send more than ~60 updates to the server, it might DC.
        // If running @144Hz every 2nd Update. If running @60Hz every update
        lastRun_ = 0;
        Connection::Run();
    }
    if (state_ == ClientState::World)
        lastPing_ += timeElapsed;
}

bool Client::HttpRequest(const std::string& path, std::ostream& out)
{
    if (httpClient_ == nullptr)
        return false;
    SimpleWeb::CaseInsensitiveMultimap header;
    header.emplace("Connection", "keep-alive");
    std::stringstream ss;
    ss << accountUuid_ << password_;
    header.emplace("Auth", ss.str());
    try
    {
        auto r = httpClient_->request("GET", path, "", header);
        if (r->status_code != "200 OK")
            return false;
        out << r->content.rdbuf();
        return true;
    }
    catch (const SimpleWeb::system_error&)
    {
        return false;
    }
}

bool Client::HttpDownload(const std::string& path, const std::string& outFile)
{
    std::remove(outFile.c_str());
    std::ofstream f;
    f.open(outFile);
    if (!f.is_open())
        return false;
    bool ret = HttpRequest(path, f);
    f.close();
    return ret;
}

uint32_t Client::GetIp() const
{
    if (protoGame_ && protoGame_->IsConnected())
        return protoGame_->GetIp();
    return 0;
}

int64_t Client::GetClockDiff() const
{
    if (protoGame_)
        return protoGame_->GetClockDiff();
    return 0;
}

void Client::ChangeMap(const std::string& mapUuid)
{
    if (state_ == ClientState::World)
        protoGame_->ChangeMap(mapUuid);
}

void Client::GetMailHeaders()
{
    if (state_ == ClientState::World)
        protoGame_->GetMailHeaders();
}

void Client::GetMail(const std::string& mailUuid)
{
    if (state_ == ClientState::World)
        protoGame_->GetMail(mailUuid);
}

void Client::DeleteMail(const std::string& mailUuid)
{
    if (state_ == ClientState::World)
        protoGame_->DeleteMail(mailUuid);
}

void Client::SendMail(const std::string& recipient, const std::string& subject, const std::string& body)
{
    if (state_ == ClientState::World)
        protoGame_->SendMail(recipient, subject, body);
}

void Client::Move(uint8_t direction)
{
    if (state_ == ClientState::World)
        protoGame_->Move(direction);
}

void Client::Turn(uint8_t direction)
{
    if (state_ == ClientState::World)
        protoGame_->Turn(direction);
}

void Client::SetDirection(float rad)
{
    if (state_ == ClientState::World)
        protoGame_->SetDirection(rad);
}

void Client::ClickObject(uint32_t sourceId, uint32_t targetId)
{
    if (state_ == ClientState::World)
        protoGame_->ClickObject(sourceId, targetId);
}

void Client::SelectObject(uint32_t sourceId, uint32_t targetId)
{
    if (state_ == ClientState::World)
        protoGame_->SelectObject(sourceId, targetId);
}

void Client::FollowObject(uint32_t targetId)
{
    if (state_ == ClientState::World)
        protoGame_->Follow(targetId);
}

void Client::Command(AB::GameProtocol::CommandTypes type, const std::string& data)
{
    if (state_ == ClientState::World)
        protoGame_->Command(type, data);
}

void Client::GotoPos(const Vec3& pos)
{
    if (state_ == ClientState::World)
        protoGame_->GotoPos(pos);
}

void Client::PartyInvitePlayer(uint32_t targetId)
{
    if (state_ == ClientState::World)
        protoGame_->PartyInvitePlayer(targetId);
}

void Client::UseSkill(uint32_t index)
{
    if (state_ == ClientState::World)
        protoGame_->UseSkill(index);
}

void Client::Cancel()
{
    if (state_ == ClientState::World)
        protoGame_->Cancel();
}

void Client::SetPlayerState(AB::GameProtocol::CreatureState newState)
{
    if (state_ == ClientState::World)
        protoGame_->SetPlayerState(newState);
}

}
