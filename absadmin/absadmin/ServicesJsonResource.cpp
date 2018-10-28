#include "stdafx.h"
#include "ServicesJsonResource.h"
#include "StringHash.h"
#include "Subsystems.h"
#include <AB/Entities/ServiceList.h>
#include <AB/Entities/Service.h>
#include "DataClient.h"
#include "Subsystems.h"
#include "ContentTypes.h"
#include "Application.h"
#include "StringUtils.h"

namespace Resources {

void ServicesJsonResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    bool loggedIn = session_->values_[Utils::StringHashRt("logged_in")].GetBool();
    if (!loggedIn)
    {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized,
            "Unauthorized");
        return;
    }

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::ServiceList sl;
    if (!dataClient->Read(sl))
    {
        response->write(SimpleWeb::StatusCode::client_error_not_found,
            "Not found");
        return;
    }

    json::JSON obj;
    obj["services"] = json::Array();
    for (const std::string& uuid : sl.uuids)
    {
        AB::Entities::Service s;
        s.uuid = uuid;
        if (!dataClient->Read(s))
            continue;
        auto serv = json::Object();
        serv["uuid"] = s.uuid;
        serv["name"] = s.name;
        serv["file"] = s.file;
        serv["host"] = s.host;
        serv["load"] = s.load;
        serv["location"] = s.location;
        serv["path"] = s.path;
        serv["port"] = s.port;
        serv["run_time"] = static_cast<long>(s.runTime);
        serv["status"] = static_cast<int>(s.status);
        serv["online"] = s.status == AB::Entities::ServiceStatusOnline;
        serv["type"] = static_cast<int>(s.type);

        obj["services"].append(serv);
    }

    SimpleWeb::CaseInsensitiveMultimap header = Application::GetDefaultHeader();
    auto contT = GetSubsystem<ContentTypes>();
    header.emplace("Content-Type", contT->Get(Utils::GetFileExt(".json")));
    responseCookies_->Write(header);
    response->write(obj.dump(), header);
}

}
