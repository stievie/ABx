#include "stdafx.h"
#include "Client.h"
#include "ProtocolLogin.h"
#include "ProtocolGame.h"
#include "Connection.h"

namespace Client {

Client::Client() :
    loginHost_("127.0.0.1"),
    gameHost_("127.0.0.1"),
    loginPort_(2748),
    gamePort_(2749),
    protoLogin_(nullptr),
    protoGame_(nullptr),
    state_(StateDisconnected),
    lastRun_(0),
    lastPing_(0),
    gotPong_(true)
{
    Crypto::DHKeys::Instance.GenerateKeys();
}

Client::~Client()
{
    Connection::Terminate();
}

void Client::OnGetCharlist(const CharList& chars)
{
    gamePort_ = protoLogin_->gamePort_;
    if (!protoLogin_->gameHost_.empty())
        gameHost_ = protoLogin_->gameHost_;
    state_ = StateSelectChar;
    if (receiver_)
        receiver_->OnGetCharlist(chars);
}

void Client::OnEnterWorld(const std::string& mapName, uint32_t playerId)
{
    state_ = StateWorld;
    mapName_ = mapName;
    if (receiver_)
        receiver_->OnEnterWorld(mapName, playerId);
}

void Client::OnDespawnObject(uint32_t id)
{
    if (receiver_)
        receiver_->OnDespawnObject(id);
}

void Client::OnObjectPos(uint32_t id, const Vec3& pos)
{
    if (receiver_)
        receiver_->OnObjectPos(id, pos);
}

void Client::OnObjectRot(uint32_t id, float rot, bool manual)
{
    if (receiver_)
        receiver_->OnObjectRot(id, rot, manual);
}

void Client::OnObjectStateChange(uint32_t id, AB::GameProtocol::CreatureState state)
{
    if (receiver_)
        receiver_->OnObjectStateChange(id, state);
}

void Client::OnAccountCreated()
{
    if (receiver_)
        receiver_->OnAccountCreated();
}

void Client::OnPlayerCreated(const std::string& name, const std::string& map)
{
    if (receiver_)
        receiver_->OnPlayerCreated(name, map);
}

void Client::OnObjectSelected(uint32_t sourceId, uint32_t targetId)
{
    if (receiver_)
        receiver_->OnObjectSelected(sourceId, targetId);
}

void Client::OnServerMessage(AB::GameProtocol::ServerMessageType type,
    const std::string& senderName, const std::string& message)
{
    if (receiver_)
        receiver_->OnServerMessage(type, senderName, message);
}

void Client::OnChatMessage(AB::GameProtocol::ChatMessageChannel channel,
    uint32_t senderId, const std::string& senderName, const std::string& message)
{
    if (receiver_)
        receiver_->OnChatMessage(channel, senderId, senderName, message);
}

void Client::OnSpawnObject(uint32_t id, const Vec3& pos, const Vec3& scale, float rot,
    PropReadStream& data, bool existing)
{
    if (receiver_)
        receiver_->OnSpawnObject(id, pos, scale, rot, data, existing);
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

void Client::Login(const std::string& name, const std::string& pass)
{
    if (!(state_ == StateDisconnected || state_ == StateCreateAccount))
        return;

    accountName_ = name;
    password_ = pass;

    // 1. Login to login server -> get character list
    protoLogin_ = std::make_shared<ProtocolLogin>();
    protoLogin_->SetErrorCallback(std::bind(&Client::OnNetworkError, this, std::placeholders::_1));
    protoLogin_->SetProtocolErrorCallback(std::bind(&Client::OnProtocolError, this, std::placeholders::_1));
    protoLogin_->Login(loginHost_, loginPort_, name, pass,
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

    protoLogin_ = std::make_shared<ProtocolLogin>();
    protoLogin_->SetErrorCallback(std::bind(&Client::OnNetworkError, this, std::placeholders::_1));
    protoLogin_->SetProtocolErrorCallback(std::bind(&Client::OnProtocolError, this, std::placeholders::_1));
    protoLogin_->CreateAccount(loginHost_, loginPort_, name, pass,
        email, accKey,
        std::bind(&Client::OnAccountCreated, this));
    Connection::Run();
}

void Client::CreatePlayer(const std::string& account, const std::string& password,
    const std::string& charName, const std::string& prof, PlayerSex sex, bool isPvp)
{
    if (state_ != StateSelectChar)
        return;

    accountName_ = account;
    password_ = password;

    protoLogin_ = std::make_shared<ProtocolLogin>();
    protoLogin_->SetErrorCallback(std::bind(&Client::OnNetworkError, this, std::placeholders::_1));
    protoLogin_->SetProtocolErrorCallback(std::bind(&Client::OnProtocolError, this, std::placeholders::_1));
    protoLogin_->CreatePlayer(loginHost_, loginPort_, account, password,
        charName, prof, sex, isPvp,
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
    }
}

void Client::EnterWorld(const std::string& charName, const std::string& map)
{
    if (state_ != StateSelectChar)
        return;
    // 2. Login to game server
    protoGame_ = std::make_shared<ProtocolGame>();
    protoGame_->receiver_ = this;

    protoGame_->Login(accountName_, password_, charName, map, gameHost_, gamePort_);
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
        Connection::Run();
        lastRun_ = 0;
    }
    lastRun_ += timeElapsed;
    if (state_ == StateWorld)
        lastPing_ += timeElapsed;
}

uint32_t Client::GetIp() const
{
    if (protoGame_ && protoGame_->IsConnected())
        return protoGame_->GetIp();
    return 0;
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

void Client::Command(AB::GameProtocol::CommandTypes type, const std::string& data)
{
    if (state_ == StateWorld)
        protoGame_->Command(type, data);
}

}
