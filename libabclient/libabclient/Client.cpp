#include "stdafx.h"
#include "Client.h"
#include "ProtocolLogin.h"
#include "ProtocolGame.h"
#include "Connection.h"

namespace Client {

Client::Client() :
    loginHost_("127.0.0.1"),
    loginPort_(2748),
    gamePort_(2749),
    protoLogin_(nullptr),
    protoGame_(nullptr),
    state_(StateDisconnected)
{
}

Client::~Client()
{
    Connection::Terminate();
}

void Client::OnGetCharlist()
{
    state_ = StateSelecChar;
    if (receiver_)
        receiver_->OnGetCharlist();
}

void Client::OnEnterWorld(const std::string& mapName)
{
    state_ = StateWorld;
    mapName_ = mapName;
    if (receiver_)
        receiver_->OnEnterWorld(mapName);
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

void Client::Login(const std::string& name, const std::string& pass)
{
    accountName_ = name;
    password_ = pass;

    // 1. Login to login server -> get character list
    protoLogin_ = std::make_shared<ProtocolLogin>();
    protoLogin_->SetErrorCallback(std::bind(&Client::OnError, this, std::placeholders::_1));
    protoLogin_->Login(loginHost_, loginPort_, name, pass,
        std::bind(&Client::OnGetCharlist, this));
    Connection::Run();
}

void Client::EnterWorld(const std::string& charName)
{
    // 2. Login to game server
    protoGame_ = std::make_shared<ProtocolGame>();
    protoGame_->SetErrorCallback(std::bind(&Client::OnError, this, std::placeholders::_1));
    protoGame_->Login(accountName_, password_, charName, loginHost_, gamePort_,
        std::bind(&Client::OnEnterWorld, this, std::placeholders::_1));
    Connection::Run();
}

void Client::Update()
{
    Connection::Poll();
}

const Charlist& Client::GetCharacters() const
{
    static Charlist empty;
    if (!protoLogin_)
        return empty;
    return protoLogin_->characters_;
}

}
