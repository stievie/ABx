#include "stdafx.h"
#include "ProtocolLogin.h"

ProtocolLogin::ProtocolLogin(const std::string& account, const std::string& password) :
    Protocol(),
    accountName_(account),
    password_(password)
{
}

ProtocolLogin::~ProtocolLogin()
{
}

void ProtocolLogin::OnConnect()
{
}

void ProtocolLogin::OnReceive(const std::shared_ptr<InputMessage>& message)
{
}
