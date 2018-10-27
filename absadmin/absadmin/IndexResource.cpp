#include "stdafx.h"
#include "IndexResource.h"
#include "Application.h"
#include "Logger.h"
#include "FileUtils.h"
#include "Subsystems.h"
#include <AB/Entities/ServiceList.h>
#include <AB/Entities/Service.h>
#include "DataClient.h"
#include "Variant.h"
#include "StringHash.h"

namespace Resources {

bool IndexResource::GetObjects(std::map<std::string, ginger::object>& objects)
{
    objects["title"] = "ABS Admin";
    auto it = session_->values_.find(Utils::StringHashRt("username"));
    if (it != session_->values_.end())
        objects["user"] = (*it).second.GetString();
    else
        objects["user"] = "";

    std::vector<std::map<std::string, ginger::object>> xs;

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::ServiceList sl;
    if (!dataClient->Read(sl))
        return false;
    for (const std::string& uuid : sl.uuids)
    {
        AB::Entities::Service s;
        s.uuid = uuid;
        if (!dataClient->Read(s))
            continue;
        xs.push_back({
            { "uuid", s.uuid },
            { "name", s.name },
        });
    }
    objects["xs"] = xs;

    return true;
}

std::string IndexResource::GetTemplate()
{
    bool loggedIn = session_->values_[Utils::StringHashRt("logged_in")].GetBool();
    if (loggedIn)
        return "../templates/index.html";
    return "../templates/login.html";
}

}
