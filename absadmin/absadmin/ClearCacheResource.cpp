#include "stdafx.h"
#include "ClearCacheResource.h"
#include "Application.h"
#include "ContentTypes.h"
#include "StringUtils.h"
#include "DataClient.h"
#include "Subsystems.h"
#include "MessageClient.h"
#include <AB/Entities/Service.h>

namespace Resources {

bool ClearCacheResource::ClearCache(const std::string& uuid)
{
    if (uuid.empty() || uuids::uuid(uuid).nil())
        return false;


    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::Service s;
    s.uuid = uuid;
    if (!dataClient->Read(s))
        return false;

    if (s.type == AB::Entities::ServiceTypeGameServer)
    {
        auto msgClient = GetSubsystem<Net::MessageClient>();
        if (!msgClient)
            return false;
        Net::MessageMsg msg;
        msg.type_ = Net::MessageType::ClearCache;
        msg.SetBodyString(uuid);
        return msgClient->Write(msg);
    }

    if (s.type == AB::Entities::ServiceTypeDataServer)
    {
        return dataClient->Clear();
    }

    return false;
}

void ClearCacheResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountTypeGod))
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
    auto uuidIt = form.find("uuid");
    if (uuidIt == form.end())
    {
        obj["status"] = "Failed";
        obj["message"] = "Missing UUID field";
    }
    else
    {
        if (!ClearCache((*uuidIt).second))
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
