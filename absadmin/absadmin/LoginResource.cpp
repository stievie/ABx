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
#include <sa/ScopeGuard.h>

namespace Resources {

LoginResource::AuthResult LoginResource::Auth(const std::string& user, const std::string& pass)
{
    uint32_t ip = request_->remote_endpoint->address().to_v4().to_uint();

    AB::Entities::Account account;
    account.name = user;
    auto dataClient = GetSubsystem<IO::DataClient>();
    if (!dataClient->Read(account))
        return AuthResult::WrongUsernamePassword;
    if (account.status != AB::Entities::AccountStatusActivated)
        return AuthResult::NotActivated;
    if (account.type < AB::Entities::AccountType::Normal)
        return AuthResult::InternalError;
    auto banMan = GetSubsystem<Auth::BanManager>();
    if (banMan->IsAccountBanned(uuids::uuid(account.uuid)))
    {
        banMan->AddLoginAttempt(ip, false);
        return AuthResult::Banned;
    }
    if (bcrypt_checkpass(pass.c_str(), account.password.c_str()) != 0)
    {
        banMan->AddLoginAttempt(ip, false);
        return AuthResult::WrongUsernamePassword;
    }
    banMan->AddLoginAttempt(ip, true);

    session_->values_[sa::StringHashRt("logged_in")] = true;
    session_->values_[sa::StringHashRt("username")] = user;
    session_->values_[sa::StringHashRt("account_uuid")] = account.uuid;
    session_->values_[sa::StringHashRt("account_type")] = static_cast<int>(account.type);

    return AuthResult::Success;
}

void LoginResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    auto contT = GetSubsystem<ContentTypes>();
    header_.emplace("Content-Type", contT->Get(".json"));

    json::JSON obj;
    sa::ScopeGuard send([&]()
    {
        Send(obj.dump(), response);
    });

    auto userIt = GetFormField("username");
    auto passIt = GetFormField("password");
    if (!userIt.has_value() || !passIt.has_value())
    {
        obj["status"] = "Failed";
        obj["message"] = "Username and/or password missing.";
        return;
    }
    auto result = Auth(userIt.value(), passIt.value());
    switch (result)
    {
    case AuthResult::Success:
        obj["status"] = "OK";
        obj["message"] = "OK";
        break;
    case AuthResult::Banned:
        obj["status"] = "Failed";
        obj["message"] = "Your account is banned.";
        break;
    case AuthResult::InternalError:
        obj["status"] = "Failed";
        obj["message"] = "Internal error";
        break;
    case AuthResult::NotActivated:
        obj["status"] = "Failed";
        obj["message"] = "This account is not activated.";
        break;
    case AuthResult::WrongUsernamePassword:
        obj["status"] = "Failed";
        obj["message"] = "Wrong Username and/or password.";
        break;
    }
}

}
