#include "stdafx.h"
#include "Client.h"

namespace Client {

Client::Client() :
    loginHost_("localhost"),
    loginPort_(2748),
    proto_(nullptr)
{
}


Client::~Client()
{
}

void Client::Login(const std::string& name, const std::string& pass)
{
    proto_ = std::make_unique<ProtocolGame>();
    proto_->Login(name, pass, loginHost_, loginPort_);
}

}
