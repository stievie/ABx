#include "stdafx.h"
#include "ServiceResource.h"
#include "Application.h"
#include "Subsystems.h"
#include <AB/Entities/ServiceList.h>
#include <AB/Entities/Service.h>
#include "DataClient.h"
#include <uuid.h>
#include "TimeUtils.h"
#include "Xml.h"

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
    objects["machine"] = Utils::XML::Escape(s.machine);
    objects["name"] = Utils::XML::Escape(s.name);
    objects["location"] = Utils::XML::Escape(s.location);
    objects["host"] = Utils::XML::Escape(s.host);
    objects["port"] = std::to_string(s.port);
    objects["ip"] = s.ip.empty() ? "0.0.0.0" : s.ip;
    objects["file"] = Utils::XML::Escape(s.file);
    objects["path"] = Utils::XML::Escape(s.path);
    objects["arguments"] = Utils::XML::Escape(s.arguments);
    objects["spawnable"] = (s.type == AB::Entities::ServiceTypeFileServer ||
        s.type == AB::Entities::ServiceTypeGameServer) && (s.status == AB::Entities::ServiceStatusOnline);
    objects["has_cache"] = (s.type == AB::Entities::ServiceTypeGameServer || s.type == AB::Entities::ServiceTypeDataServer) &&
        (s.status == AB::Entities::ServiceStatusOnline);
    objects["online"] = s.status == AB::Entities::ServiceStatusOnline;
    // Temporary are always running
    objects["termable"] = s.temporary;
    Utils::TimeSpan ts(static_cast<uint32_t>(s.runTime));
    std::stringstream ss;
    if (ts.months > 0)
        ss << ts.months << " month(s) ";
    if (ts.days > 0)
        ss << ts.days << " day(s) ";
    ss << ts.hours << "h " << ts.minutes << "m " << ts.seconds << "s";

    objects["uptime"] = Utils::XML::Escape(ss.str());

    return true;
}

ServiceResource::ServiceResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request),
    id_("")
{
    template_ = "../templates/service.html";

    footerScripts_.push_back("vendors/gauge.js/dist/gauge.min.js");

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
