#include "stdafx.h"
#include "Controller.h"
#include <uuid.h>
#include "Application.h"
#include "Subsystems.h"
#include "Sessions.h"

void Controller::MakeRequest(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
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
    std::shared_ptr<HTTP::Session> sess = sessions->Get(sessId);
    if (!sess)
    {
        // Create a session with this ID
        sess = std::make_shared<HTTP::Session>();
        sessions->Add(sessId, sess);
    }
    session_ = sess;
    HTTP::Cookie respSessCookie;
    respSessCookie.content_ = sessId;
    respSessCookie.domain_ = Application::Instance->GetHost();
    responseCookies_->Add("SESSION_ID", respSessCookie);
}
