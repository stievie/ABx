#include "stdafx.h"
#include "PasswordPostResource.h"
#include "Application.h"
#include "Subsystems.h"
#include "ContentTypes.h"
#include "StringUtils.h"
#include "DataClient.h"
#include "Subsystems.h"
#include <abcrypto.hpp>

namespace Resources {

bool PasswordPostResource::ChangePassword(const SimpleWeb::CaseInsensitiveMultimap& form, std::string& error)
{
    auto oldPwIt = form.find("old_password");
    auto newPwIt = form.find("new_password");
    auto new2PwIt = form.find("new_password2");
    if (oldPwIt == form.end() || newPwIt == form.end() || new2PwIt == form.end())
        return false;

    std::string uuid = session_->values_[Utils::StringHashRt("account_uuid")].GetString();
    AB::Entities::Account account;
    account.uuid = uuid;
    auto dataClient = GetSubsystem<IO::DataClient>();
    if (!dataClient->Read(account))
    {
        error = "Invalid Account";
        return false;
    }

    if (bcrypt_checkpass((*oldPwIt).second.c_str(), account.password.c_str()) != 0)
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
    if (bcrypt_newhash((*newPwIt).second.c_str(), 10, pwhash, 61) != 0)
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
    if (!IsAllowed(AB::Entities::AccountTypeNormal))
    {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized,
            "Unauthorized");
        return;
    }

    SimpleWeb::CaseInsensitiveMultimap header = Application::GetDefaultHeader();
    auto contT = GetSubsystem<ContentTypes>();
    header.emplace("Content-Type", contT->Get(Utils::GetFileExt(".json")));
    responseCookies_->Write(header);

    std::stringstream ss;
    ss << request_->content.rdbuf();

    json::JSON obj;
    SimpleWeb::CaseInsensitiveMultimap form = SimpleWeb::QueryString::parse(ss.str());
    std::string error;
    if (ChangePassword(form, error))
        obj["status"] = "OK";
    else
    {
        obj["status"] = "Failed";
        obj["message"] = error;
    }

    response->write(obj.dump(), header);
}

}