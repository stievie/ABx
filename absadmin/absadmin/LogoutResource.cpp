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
    SimpleWeb::CaseInsensitiveMultimap header = Application::GetDefaultHeader();
    auto contT = GetSubsystem<ContentTypes>();
    header.emplace("Content-Type", contT->Get(Utils::GetFileExt(".json")));
    responseCookies_->Write(header);

    std::stringstream ss;
    ss << request_->content.rdbuf();

    SimpleWeb::CaseInsensitiveMultimap form = SimpleWeb::QueryString::parse(ss.str());

    session_->values_[Utils::StringHashRt("logged_in")] = false;

    json::JSON obj;
    obj["status"] = "OK";

    response->write(obj.dump(), header);
}

}
