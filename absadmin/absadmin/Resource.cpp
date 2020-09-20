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

#include "Resource.h"
#include <uuid.h>
#include "Application.h"
#include <abscommon/SimpleConfigManager.h>
#include <sa/StringHash.h>

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
    bool loggedIn = session_->values_[sa::StringHashRt("logged_in")].GetBool();
    if (!loggedIn)
    {
        return minType == AB::Entities::AccountType::Unknown;
    }
    auto accIt = session_->values_.find(sa::StringHashRt("account_type"));
    AB::Entities::AccountType accType = AB::Entities::AccountType::Unknown;
    if (accIt != session_->values_.end())
        accType = static_cast<AB::Entities::AccountType>((*accIt).second.GetInt());

    return accType >= minType;
}

Resource::Resource(std::shared_ptr<HttpsServer::Request> request) :
    request_(request),
    header_(Application::GetDefaultHeader())
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
    auto* config = GetSubsystem<IO::SimpleConfigManager>();

    HTTP::Cookie* respSessCookie = responseCookies_->Add("SESSION_ID");
    respSessCookie->content_ = sessId;
    respSessCookie->expires_ = session_->expires_;
    respSessCookie->domain_ = Application::Instance->GetHost();
    respSessCookie->httpOnly_ = true;
    respSessCookie->sameSite_ = static_cast<HTTP::Cookie::SameSite>(config->GetGlobalInt("cookie_samesite", 0));
    respSessCookie->secure_ = config->GetGlobalBool("cookie_secure", false);
}

}
