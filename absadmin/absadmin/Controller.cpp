#include "stdafx.h"
#include "Controller.h"
#include <uuid.h>
#include "Application.h"
#include "Subsystems.h"
#include "Sessions.h"

void Controller::MakeRequest(std::shared_ptr<HttpsServer::Response> response,
    std::shared_ptr<HttpsServer::Request> request)
{
    responseCookies_ = std::make_unique<Cookies>();
    requestCookies_ = std::make_unique<Cookies>(*request);
    Cookie* sessCookie = requestCookies_->Get("SESSION_ID");
    std::string sessId;
    if (sessCookie == nullptr)
    {
        const uuids::uuid guid = uuids::uuid_system_generator{}();
        sessId = guid.to_string();
    }
    else
        sessId = sessCookie->content_;

    auto sessions = GetSubsystem<Sessions>();
    auto sess = sessions->Get(sessId);
    if (!sess)
    {
        sess = std::make_shared<Utils::VariantMap>();
        sessions->Add(sessId, sess);
        // No session -> also create new Session ID
        const uuids::uuid guid = uuids::uuid_system_generator{}();
        sessId = guid.to_string();
    }
    session_ = sess;
    Cookie respSessCookie;
    respSessCookie.content_ = sessId;
    respSessCookie.domain_ = Application::Instance->GetHost();
    responseCookies_->Add("SESSION_ID", respSessCookie);
}
