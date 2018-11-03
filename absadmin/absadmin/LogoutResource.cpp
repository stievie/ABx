#include "stdafx.h"
#include "LogoutResource.h"
#include "Subsystems.h"
#include "Application.h"
#include "ContentTypes.h"
#include "StringUtils.h"
#include "StringHash.h"

namespace Resources {

void LogoutResource::Render(std::shared_ptr<HttpsServer::Response> response)
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

    session_->values_[Utils::StringHashRt("logged_in")] = false;

    json::JSON obj;
    obj["status"] = "OK";

    response->write(obj.dump(), header);
}

}
