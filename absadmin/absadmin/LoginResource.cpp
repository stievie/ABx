#include "stdafx.h"
#include "LoginResource.h"
#include "Application.h"
#include "Subsystems.h"
#include "StringUtils.h"
#include "ContentTypes.h"
#include "Logger.h"
#include "DataClient.h"
#include <AB/Entities/Account.h>
#include <abcrypto.hpp>
#include "BanManager.h"

namespace Resources {

bool LoginResource::Auth(const std::string& user, const std::string& pass)
{
    uint32_t ip = request_->remote_endpoint->address().to_v4().to_uint();

    AB::Entities::Account account;
    account.name = user;
    auto dataClient = GetSubsystem<IO::DataClient>();
    if (!dataClient->Read(account))
        return false;
    if (account.status != AB::Entities::AccountStatusActivated)
        return false;
    if (account.type < AB::Entities::AccountTypeNormal)
        return false;
    auto banMan = GetSubsystem<Auth::BanManager>();
    if (banMan->IsAccountBanned(uuids::uuid(account.uuid)))
    {
        banMan->AddLoginAttempt(ip, false);
        return false;
    }
    if (bcrypt_checkpass(pass.c_str(), account.password.c_str()) != 0)
    {
        banMan->AddLoginAttempt(ip, false);
        return false;
    }
    banMan->AddLoginAttempt(ip, true);

    session_->values_[Utils::StringHashRt("logged_in")] = true;
    session_->values_[Utils::StringHashRt("username")] = user;
    session_->values_[Utils::StringHashRt("account_uuid")] = account.uuid;
    session_->values_[Utils::StringHashRt("account_type")] = static_cast<int>(account.type);

    return true;
}

void LoginResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    SimpleWeb::CaseInsensitiveMultimap header = Application::GetDefaultHeader();
    auto contT = GetSubsystem<ContentTypes>();
    header.emplace("Content-Type", contT->Get(".json"));
    responseCookies_->Write(header);

    std::stringstream ss;
    ss << request_->content.rdbuf();

    SimpleWeb::CaseInsensitiveMultimap form = SimpleWeb::QueryString::parse(ss.str());

    json::JSON obj;

    auto userIt = form.find("username");
    auto passIt = form.find("password");
    if (userIt == form.end() || passIt == form.end())
    {
        obj["status"] = "Failed";
    }
    else
    {
        if (Auth((*userIt).second, (*passIt).second))
            obj["status"] = "OK";
        else
            obj["status"] = "Failed";
    }

    response->write(obj.dump(), header);
}

}
