#include "stdafx.h"
#include "SpawnResource.h"
#include "Application.h"
#include "Subsystems.h"
#include "ContentTypes.h"
#include "StringUtils.h"
#include "DataClient.h"
#include "Subsystems.h"
#include "MessageClient.h"

namespace Resources {

bool SpawnResource::Spawn(const std::string& uuid)
{
    auto msgClient = GetSubsystem<Net::MessageClient>();
    if (!msgClient)
        return false;
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::Spawn;
    msg.SetBodyString(uuid);
    return msgClient->Write(msg);
}

void SpawnResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
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
        if (!Spawn((*uuidIt).second))
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
