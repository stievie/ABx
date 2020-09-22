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


#include "ServicesResource.h"
#include "Application.h"
#include "Version.h"
#include <AB/Entities/ServiceList.h>
#include <AB/Entities/Service.h>

namespace Resources {

bool ServicesResource::GetContext(LuaContext& objects)
{
    if (!TemplateResource::GetContext(objects))
        return false;

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::ServiceList sl;
    if (!dataClient->Read(sl))
        return false;

    kaguya::State& state = objects.GetState();
    state["services"] = kaguya::NewTable();
    int i = 0;
    for (const std::string& uuid : sl.uuids)
    {
        AB::Entities::Service s;
        s.uuid = uuid;
        if (!dataClient->Read(s))
            continue;

        state["services"][++i] = kaguya::TableData{
            { "uuid", s.uuid },
            { "name", Utils::XML::Escape(s.name) },
            { "file", Utils::XML::Escape(s.file) },
            { "host", Utils::XML::Escape(s.host) },
            { "load", std::to_string(s.load) },
            { "location", Utils::XML::Escape(s.location) },
            { "path", Utils::XML::Escape(s.path) },
            { "port", s.port },
            { "run_time", s.runTime },
            { "start_time", s.startTime },
            { "stop_time", s.stopTime },
            { "status", s.status },
            { "online", (bool)(s.status == AB::Entities::ServiceStatusOnline) },
            { "type", s.type }
        };
    }

    return true;
}

ServicesResource::ServicesResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request)
{
    template_ = "../templates/services.lpp";

    footerScripts_.push_back("vendors/js/gauge.min.js");
}

void ServicesResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountType::God))
    {
        Redirect(response, "/");
        return;
    }

    TemplateResource::Render(response);
}

}
