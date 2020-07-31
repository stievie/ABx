/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "ServiceResource.h"
#include "Application.h"
#include <AB/Entities/ServiceList.h>
#include <AB/Entities/Service.h>
#include <uuid.h>

namespace Resources {

bool ServiceResource::GetContext(LuaContext& objects)
{
    if (!TemplateResource::GetContext(objects))
        return false;

    if (id_.empty() || uuids::uuid(id_).nil())
        return false;

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::Service s;
    s.uuid = id_;
    if (!dataClient->Read(s))
        return false;

    kaguya::State& state = objects.GetState();

    state["uuid"] = s.uuid;
    state["machine"] = Utils::XML::Escape(s.machine);
    state["name"] = Utils::XML::Escape(s.name);
    state["location"] = Utils::XML::Escape(s.location);
    state["host"] = Utils::XML::Escape(s.host);
    state["port"] = std::to_string(s.port);
    state["ip"] = s.ip.empty() ? "0.0.0.0" : s.ip;
    state["file"] = Utils::XML::Escape(s.file);
    state["path"] = Utils::XML::Escape(s.path);
    state["arguments"] = Utils::XML::Escape(s.arguments);
    state["spawnable"] = (s.type == AB::Entities::ServiceTypeFileServer ||
        s.type == AB::Entities::ServiceTypeGameServer) && (s.status == AB::Entities::ServiceStatusOnline);
    state["has_cache"] = (s.type == AB::Entities::ServiceTypeGameServer || s.type == AB::Entities::ServiceTypeDataServer) &&
        (s.status == AB::Entities::ServiceStatusOnline);
    state["online"] = s.status == AB::Entities::ServiceStatusOnline;
    // Temporary are always running
    state["termable"] = s.temporary;
    Utils::TimeSpan ts(static_cast<uint32_t>(s.runTime));
    std::stringstream ss;
    if (ts.months > 0)
        ss << ts.months << " month(s) ";
    if (ts.days > 0)
        ss << ts.days << " day(s) ";
    ss << ts.hours << "h " << ts.minutes << "m " << ts.seconds << "s";

    state["uptime"] = Utils::XML::Escape(ss.str());

    return true;
}

ServiceResource::ServiceResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request),
    id_("")
{
    template_ = "../templates/service.lpp";

    footerScripts_.push_back("vendors/gauge.js/dist/gauge.min.js");

    SimpleWeb::CaseInsensitiveMultimap form = request->parse_query_string();
    auto it = form.find("id");
    if (it != form.end())
        id_ = (*it).second;
}

void ServiceResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountType::God))
    {
        Redirect(response, "/");
        return;
    }

    TemplateResource::Render(response);
}

}
