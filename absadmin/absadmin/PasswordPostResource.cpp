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


#include "PasswordPostResource.h"
#include "Application.h"
#include "ContentTypes.h"
#include <abcrypto.hpp>
#include <sa/StringHash.h>

namespace Resources {

bool PasswordPostResource::ChangePassword(std::string& error)
{
    auto oldPwIt = GetFormField("old_password");
    auto newPwIt = GetFormField("new_password");
    auto new2PwIt = GetFormField("new_password2");
    if (!oldPwIt.has_value() || !newPwIt.has_value() || !new2PwIt.has_value())
        return false;

    std::string uuid = session_->values_[sa::StringHashRt("account_uuid")].GetString();
    AB::Entities::Account account;
    account.uuid = uuid;
    auto dataClient = GetSubsystem<IO::DataClient>();
    if (!dataClient->Read(account))
    {
        error = "Invalid Account";
        return false;
    }

    if (bcrypt_checkpass(oldPwIt.value().c_str(), account.password.c_str()) != 0)
    {
        error = "Password check failed";
        return false;
    }
    if ((*newPwIt) != (*new2PwIt))
    {
        error = "Password repeat check failed";
        return false;
    }

    // Create the account
    char pwhash[61];
    if (bcrypt_newhash(newPwIt.value().c_str(), 10, pwhash, 61) != 0)
    {
        LOG_ERROR << "bcrypt_newhash() failed" << std::endl;
        error = "Don't ask!";
        return false;
    }
    std::string passwordHash(pwhash, 61);
    account.password = passwordHash;
    if (!dataClient->Update(account))
    {
        error = "Update failed";
        return false;
    }

    return true;
}

void PasswordPostResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountType::Normal))
    {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized,
            "Unauthorized");
        return;
    }

    auto contT = GetSubsystem<ContentTypes>();
    header_.emplace("Content-Type", contT->Get(".json"));

    json::JSON obj;
    std::string error;
    if (ChangePassword(error))
        obj["status"] = "OK";
    else
    {
        obj["status"] = "Failed";
        obj["message"] = error;
    }

    Send(obj.dump(), response);
}

}
