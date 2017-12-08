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

void Client::OnSpawnObject(uint32_t id, float x, float y, float z, float rot, PropReadStream& data)
{
    if (receiver_)
        receiver_->OnSpawnObject(id, x, y, z, rot, data);
}

void Client::OnError(const std::error_code& err)
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
    protoLogin_->SetErrorCallback(std::bind(&Client::OnError, this, std::placeholders::_1));
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
    protoGame_->SetErrorCallback(std::bind(&Client::OnError, this, std::placeholders::_1));
    protoGame_->SetProtocolErrorCallback(std::bind(&Client::OnProtocolError, this, std::placeholders::_1));
    protoGame_->SetSpawnCallback(std::bind(&Client::OnSpawnObject, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    protoGame_->SetDespawnCallback(std::bind(&Client::OnDespawnObject, this, std::placeholders::_1));
    protoGame_->Login(accountName_, password_, charName, map, gameHost_, gamePort_,
        std::bind(&Client::OnEnterWorld, this, std::placeholders::_1, std::placeholders::_2));
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

}
