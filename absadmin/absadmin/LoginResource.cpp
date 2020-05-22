/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "LoginResource.h"
#include "Application.h"
#include "ContentTypes.h"
#include <AB/Entities/Account.h>
#include <abcrypto.hpp>
#include <abscommon/BanManager.h>
#include <sa/StringHash.h>

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
    if (account.type < AB::Entities::AccountType::Normal)
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

    session_->values_[sa::StringHashRt("logged_in")] = true;
    session_->values_[sa::StringHashRt("username")] = user;
    session_->values_[sa::StringHashRt("account_uuid")] = account.uuid;
    session_->values_[sa::StringHashRt("account_type")] = static_cast<int>(account.type);

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
