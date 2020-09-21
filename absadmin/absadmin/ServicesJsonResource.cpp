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


#include "ServicesJsonResource.h"
#include <AB/Entities/ServiceList.h>
#include <AB/Entities/Service.h>
#include "ContentTypes.h"
#include "Application.h"

namespace Resources {

void ServicesJsonResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountType::God))
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
    obj["uuids"] = json::Array();
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

        obj["uuids"].append(s.uuid);
        obj["services"].append(serv);
    }

    auto contT = GetSubsystem<ContentTypes>();
    header_.emplace("Content-Type", contT->Get(".json"));
    Send(obj.dump(), response);
}

}
