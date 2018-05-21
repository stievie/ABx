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
    gameHost_("127.0.0.1"),
    fileHost_("127.0.0.1"),
    loginPort_(2748),
    gamePort_(2749),
    filePort_(8081),
    protoLogin_(nullptr),
    protoGame_(nullptr),
    state_(StateDisconnected),
    lastRun_(0),
    lastPing_(0),
    gotPong_(true),
    httpClient_(nullptr)
{
}

Client::~Client()
{
    if (httpClient_)
        delete httpClient_;
    Connection::Terminate();
}

void Client::OnLoggedIn(const std::string& accountUuid)
{
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
    accountUuid_ = accountUuid;
    if (receiver_)
        receiver_->OnLoggedIn(accountUuid);
}

void Client::OnGetCharlist(const AB::Entities::CharacterList& chars)
{
    state_ = StateSelectChar;
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

void Client::OnEnterWorld(int64_t updateTick, const std::string& mapUuid, uint32_t playerId)
{
    state_ = StateWorld;
    mapUuid_ = mapUuid;
    if (receiver_)
        receiver_->OnEnterWorld(updateTick, mapUuid, playerId);
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

void Client::OnSpawnObject(int64_t updateTick, uint32_t id, const Vec3& pos, const Vec3& scale, float rot,
    PropReadStream& data, bool existing)
{
    if (receiver_)
        receiver_->OnSpawnObject(updateTick, id, pos, scale, rot, data, existing);
}

void Client::OnNetworkError(const std::error_code& err)
{
    if (receiver_)
        receiver_->OnNetworkError(err);
}

void Client::OnProtocolError(uint8_t err)
{
    if (receiver_)
        receiver_->OnProtocolError(err);
}

void Client::OnPong(int ping)
{
    gotPong_ = true;
    while (pings_.size() > 9)
        pings_.erase(pings_.begin());
    pings_.push_back(ping);
}

std::shared_ptr<ProtocolLogin> Client::GetProtoLogin()
{
    if (!protoLogin_)
    {
        protoLogin_ = std::make_shared<ProtocolLogin>();
        protoLogin_->SetErrorCallback(std::bind(&Client::OnNetworkError, this, std::placeholders::_1));
        protoLogin_->SetProtocolErrorCallback(std::bind(&Client::OnProtocolError, this, std::placeholders::_1));
    }
    return protoLogin_;
}

void Client::Login(const std::string& name, const std::string& pass)
{
    if (!(state_ == StateDisconnected || state_ == StateCreateAccount))
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
    if (state_ != StateCreateAccount)
        return;

    accountName_ = name;
    password_ = pass;

    GetProtoLogin()->CreateAccount(loginHost_, loginPort_, name, pass,
        email, accKey,
        std::bind(&Client::OnAccountCreated, this));
    Connection::Run();
}

void Client::CreatePlayer(const std::string& account, const std::string& password,
    const std::string& charName, const std::string& profUuid,
    AB::Entities::CharacterSex sex, bool isPvp)
{
    if (state_ != StateSelectChar)
        return;

    accountName_ = account;
    password_ = password;

    GetProtoLogin()->CreatePlayer(loginHost_, loginPort_, account, password,
        charName, profUuid, sex, isPvp,
        std::bind(&Client::OnPlayerCreated, this, std::placeholders::_1, std::placeholders::_2));
    Connection::Run();
}

void Client::Logout()
{
    if (state_ != StateWorld)
        return;
    if (protoGame_)
    {
        protoGame_->Logout();
        Connection::Run();
        state_ = StateDisconnected;
        protoGame_.reset();
    }
}

void Client::GetOutposts()
{
    if (accountName_.empty() || password_.empty())
        return;

    GetProtoLogin()->GetOutposts(loginHost_, loginPort_, accountName_, password_,
        std::bind(&Client::OnGetOutposts, this, std::placeholders::_1));
    Connection::Run();
}

void Client::EnterWorld(const std::string& charUuid, const std::string& mapUuid)
{
    // Enter or changing the world
    if (state_ != StateSelectChar && state_ != StateWorld)
        return;

    if (state_ == StateWorld)
        // We are already logged in to some world so we must logout
        Logout();

    // 2. Login to game server
    protoGame_ = std::make_shared<ProtocolGame>();
    protoGame_->receiver_ = this;

    if (state_ == StateSelectChar && protoLogin_)
        // If we came from the select character scene
        protoLogin_.reset();

    protoGame_->Login(accountName_, password_, charUuid, mapUuid, gameHost_, gamePort_);
}

void Client::Update(int timeElapsed)
{
    if (state_ == StateWorld)
    {
        if (lastPing_ >= 1000 && gotPong_)
        {
            if (protoGame_)
            {
                gotPong_ = false;
                protoGame_->Ping(std::bind(&Client::OnPong, this, std::placeholders::_1));
            }
            lastPing_ = 0;
        }
    }

    if (lastRun_ >= 16)
    {
        // Don't send more than 60 updates to the server, it might DC.
        Connection::Run();
        lastRun_ = 0;
    }
    lastRun_ += timeElapsed;
    if (state_ == StateWorld)
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

void Client::GetMailHeaders()
{
    if (state_ == StateWorld)
        protoGame_->GetMailHeaders();
}

void Client::GetMail(const std::string& mailUuid)
{
    if (state_ == StateWorld)
        protoGame_->GetMail(mailUuid);
}

void Client::DeleteMail(const std::string& mailUuid)
{
    if (state_ == StateWorld)
        protoGame_->DeleteMail(mailUuid);
}

void Client::Move(uint8_t direction)
{
    if (state_ == StateWorld)
        protoGame_->Move(direction);
}

void Client::Turn(uint8_t direction)
{
    if (state_ == StateWorld)
        protoGame_->Turn(direction);
}

void Client::SetDirection(float rad)
{
    if (state_ == StateWorld)
        protoGame_->SetDirection(rad);
}

void Client::SelectObject(uint32_t sourceId, uint32_t targetId)
{
    if (state_ == StateWorld)
        protoGame_->SelectObject(sourceId, targetId);
}

void Client::FollowObject(uint32_t targetId)
{
    if (state_ == StateWorld)
        protoGame_->Follow(targetId);
}

void Client::Command(AB::GameProtocol::CommandTypes type, const std::string& data)
{
    if (state_ == StateWorld)
        protoGame_->Command(type, data);
}

void Client::GotoPos(const Vec3& pos)
{
    if (state_ == StateWorld)
        protoGame_->GotoPos(pos);
}

}
