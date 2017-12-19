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
    state_(StateDisconnected)
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
    state_ = StateSelecChar;
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

void Client::OnObjectRot(uint32_t id, float rot)
{
    if (receiver_)
        receiver_->OnObjectRot(id, rot);
}

void Client::OnObjectStateChange(uint32_t id, AB::GameProtocol::CreatureState state)
{
    if (receiver_)
        receiver_->OnObjectStateChange(id, state);
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
    while (pings_.size() > 9)
        pings_.erase(pings_.begin());
    pings_.push_back(ping);
}

void Client::Login(const std::string& name, const std::string& pass)
{
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

void Client::Logout()
{
    if (protoGame_)
    {
        protoGame_->Logout();
        Connection::Run();
    }
}

void Client::EnterWorld(const std::string& charName, const std::string& map)
{
    // 2. Login to game server
    protoGame_ = std::make_shared<ProtocolGame>();
    protoGame_->receiver_ = this;

    protoGame_->Login(accountName_, password_, charName, map, gameHost_, gamePort_);
}

void Client::Update(int timeElapsed)
{
    static int lastTime = 0;
    if (lastTime >= 1000)
    {
        if (protoGame_)
        {
            protoGame_->Ping(std::bind(&Client::OnPong, this, std::placeholders::_1));
        }
        lastTime = 0;
    }
    lastTime += timeElapsed;
    Connection::Run();
}

void Client::Move(uint8_t direction)
{
    protoGame_->Move(direction);
}

void Client::Turn(uint8_t direction)
{
    protoGame_->Turn(direction);
}

void Client::SetDirection(float rad)
{
    protoGame_->SetDirection(rad);
}

}
