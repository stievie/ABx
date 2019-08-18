#include "stdafx.h"
#include "Resource.h"
#include <uuid.h>
#include "Subsystems.h"
#include "Application.h"
#include "StringHash.h"
#include "UuidUtils.h"

namespace Resources {

void Resource::Redirect(std::shared_ptr<HttpsServer::Response> response, const std::string& url)
{
    auto header = Application::Instance->GetDefaultHeader();
    header.emplace("Location", url);
    responseCookies_->Write(header);
    response->write(SimpleWeb::StatusCode::redirection_found, "Found", header);
}

bool Resource::IsAllowed(AB::Entities::AccountType minType)
{
    bool loggedIn = session_->values_[Utils::StringHashRt("logged_in")].GetBool();
    if (!loggedIn)
    {
        return minType == AB::Entities::AccountTypeUnknown;
    }
    auto accIt = session_->values_.find(Utils::StringHashRt("account_type"));
    AB::Entities::AccountType accType = AB::Entities::AccountTypeUnknown;
    if (accIt != session_->values_.end())
        accType = static_cast<AB::Entities::AccountType>((*accIt).second.GetInt());

    return accType >= minType;
}

Resource::Resource(std::shared_ptr<HttpsServer::Request> request) :
    request_(request)
{
    responseCookies_ = std::make_unique<HTTP::Cookies>();
    requestCookies_ = std::make_unique<HTTP::Cookies>(*request);
    HTTP::Cookie* sessCookie = requestCookies_->Get("SESSION_ID");
    std::string sessId;
    if (sessCookie == nullptr)
        sessId = Utils::Uuid::New();
    else
        sessId = sessCookie->content_;

    auto sessions = GetSubsystem<HTTP::Sessions>();
    session_ = sessions->Get(sessId);
    if (!session_)
    {
        // Create a session with this ID
        session_ = std::make_shared<HTTP::Session>();
        sessions->Add(sessId, session_);
    }
    // Update expiry date/time
    session_->Touch();
    HTTP::Cookie respSessCookie;
    respSessCookie.content_ = sessId;
    respSessCookie.expires_ = session_->expires_;
    respSessCookie.domain_ = Application::Instance->GetHost();
    respSessCookie.httpOnly_ = true;
    responseCookies_->Add("SESSION_ID", respSessCookie);
}

}
