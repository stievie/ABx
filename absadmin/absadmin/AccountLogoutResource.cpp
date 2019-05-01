#include "stdafx.h"
#include "AccountLogoutResource.h"
#include "Application.h"
#include "ContentTypes.h"
#include "StringUtils.h"
#include "DataClient.h"
#include "Subsystems.h"
#include "MessageClient.h"

namespace Resources {

bool AccountLogoutResource::Logout(const std::string& uuid)
{
    if (uuid.empty() || uuids::uuid(uuid).nil())
        return false;

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::Account a;
    a.uuid = uuid;
    if (!dataClient->Read(a))
        return false;
    a.onlineStatus = AB::Entities::OnlineStatusOffline;
    if (!dataClient->Update(a))
        return false;
    return true;
}

void AccountLogoutResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountTypeGod))
    {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized,
            "Unauthorized");
        return;
    }

    SimpleWeb::CaseInsensitiveMultimap header = Application::GetDefaultHeader();
    auto contT = GetSubsystem<ContentTypes>();
    header.emplace("Content-Type", contT->Get(".json"));
    responseCookies_->Write(header);

    std::stringstream ss;
    ss << request_->content.rdbuf();

    json::JSON obj;
    SimpleWeb::CaseInsensitiveMultimap form = SimpleWeb::QueryString::parse(ss.str());
    auto uuidIt = form.find("uuid");
    if (uuidIt == form.end())
    {
        obj["status"] = "Failed";
        obj["message"] = "Missing UUID field";
    }
    else
    {
        if (!Logout((*uuidIt).second))
        {
            obj["status"] = "Failed";
            obj["message"] = "Failed";
        }
        else
        {
            obj["status"] = "OK";
        }
    }

    response->write(obj.dump(), header);
}

}
