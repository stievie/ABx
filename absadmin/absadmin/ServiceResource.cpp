#include "stdafx.h"
#include "ServiceResource.h"
#include "StringHash.h"
#include "Application.h"
#include "Subsystems.h"
#include <AB/Entities/ServiceList.h>
#include <AB/Entities/Service.h>
#include "DataClient.h"
#include <uuid.h>

namespace Resources {

bool ServiceResource::GetObjects(std::map<std::string, ginger::object>& objects)
{
    if (!TemplateResource::GetObjects(objects))
        return false;

    if (id_.empty() || uuids::uuid(id_).nil())
        return false;

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::Service s;
    s.uuid = id_;
    if (!dataClient->Read(s))
        return false;

    objects["uuid"] = s.uuid;
    objects["machine"] = s.machine;
    objects["name"] = s.name;
    objects["location"] = s.location;
    objects["host"] = s.host;
    objects["port"] = std::to_string(s.port);
    objects["file"] = s.file;
    objects["path"] = s.path;
    objects["arguments"] = s.arguments;
    objects["spawnable"] = (s.type == AB::Entities::ServiceTypeFileServer ||
        s.type == AB::Entities::ServiceTypeGameServer) && (s.status == AB::Entities::ServiceStatusOnline);
    objects["online"] = s.status == AB::Entities::ServiceStatusOnline;
    // Temporary are always running
    objects["termable"] = s.temporary;

    return true;
}

ServiceResource::ServiceResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request),
    id_("")
{
    template_ = "../templates/service.html";

    // Bootstrap
    footerScripts_.push_back("vendors/bootstrap/dist/js/bootstrap.min.js");
    footerScripts_.push_back("vendors/gauge.js/dist/gauge.min.js");
    footerScripts_.push_back("js/common.js");

    SimpleWeb::CaseInsensitiveMultimap form = request->parse_query_string();
    auto it = form.find("id");
    if (it != form.end())
        id_ = (*it).second;
}

void ServiceResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountTypeGod))
    {
        Redirect(response, "/");
        return;
    }

    TemplateResource::Render(response);
}

}
