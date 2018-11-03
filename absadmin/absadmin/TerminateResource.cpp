#include "stdafx.h"
#include "TerminateResource.h"
#include "Application.h"
#include "ContentTypes.h"
#include "StringUtils.h"
#include "DataClient.h"
#include "Subsystems.h"
#include "MessageClient.h"

namespace Resources {

bool TerminateResource::Terminate(const std::string& uuid)
{
    if (uuid.empty() || uuids::uuid(uuid).nil())
        return false;

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::Service s;
    s.uuid = uuid;
    if (!dataClient->Read(s))
        return false;
    // Can only terminate temporary services
    if (!s.temporary)
        return false;

    auto msgClient = GetSubsystem<Net::MessageClient>();
    if (!msgClient)
        return false;
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::Shutdown;
    msg.SetBodyString(uuid);
    return msgClient->Write(msg);
}

void TerminateResource::Render(std::shared_ptr<HttpsServer::Response> response)
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
        if (!Terminate((*uuidIt).second))
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