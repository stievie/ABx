#include "stdafx.h"
#include "Resource.h"
#include <uuid.h>
#include "Subsystems.h"
#include "Application.h"

namespace Resources {

Resource::Resource(std::shared_ptr<HttpsServer::Request> request) :
    request_(request)
{
    responseCookies_ = std::make_unique<HTTP::Cookies>();
    requestCookies_ = std::make_unique<HTTP::Cookies>(*request);
    HTTP::Cookie* sessCookie = requestCookies_->Get("SESSION_ID");
    std::string sessId;
    if (sessCookie == nullptr)
    {
        const uuids::uuid guid = uuids::uuid_system_generator{}();
        sessId = guid.to_string();
    }
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
