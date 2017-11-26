#include "stdafx.h"
#include "Client.h"
#include "ProtocolLogin.h"
#include "ProtocolGame.h"

namespace Client {

Client::Client() :
    loginHost_("127.0.0.1"),
    loginPort_(2748),
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
}

void Client::OnEnterWorld()
{
    state_ = StateWorld;
}

void Client::Login(const std::string& name, const std::string& pass)
{
    accountName_ = name;
    password_ = pass;

    // 1. Login to login server -> get character list
    protoLogin_ = std::make_shared<ProtocolLogin>();
    protoLogin_->Login(loginHost_, loginPort_, name, pass,
        std::bind(&Client::OnGetCharlist, this));
    Connection::Run();
}

void Client::EnterWorld(const std::string& charName)
{
    // 2. Login to game server
    protoGame_ = std::make_shared<ProtocolGame>();
    protoGame_->Login(accountName_, password_, charName, loginHost_, loginPort_,
        std::bind(&Client::OnEnterWorld, this));
    Connection::Run();
}

const Charlist& Client::GetCharacters() const
{
    static Charlist empty;
    if (!protoLogin_)
        return empty;
    return protoLogin_->characters_;
}

}
