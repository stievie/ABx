#include "stdafx.h"
#include "IndexResource.h"
#include "Application.h"
#include "Logger.h"
#include "FileUtils.h"
#include "Variant.h"
#include "StringHash.h"
#include "Version.h"
#include <AB/Entities/Account.h>
#include <AB/Entities/AccountList.h>
#include <AB/Entities/CharacterList.h>
#include "Subsystems.h"
#include "DataClient.h"

namespace Resources {

bool IndexResource::GetObjects(std::map<std::string, ginger::object>& objects)
{
    if (!TemplateResource::GetObjects(objects))
        return false;
    bool loggedIn = session_->values_[Utils::StringHashRt("logged_in")].GetBool();
    if (!loggedIn)
        return true;

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::AccountList al;
    if (!dataClient->Read(al))
        return false;
    objects["total_accounts"] = al.uuids.size();

    AB::Entities::CharacterList cl;
    if (!dataClient->Read(cl))
        return false;
    objects["total_chars"] = cl.uuids.size();

    return true;
}

IndexResource::IndexResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request)
{
    bool loggedIn = session_->values_[Utils::StringHashRt("logged_in")].GetBool();
    if (loggedIn)
        template_ = "../templates/dashboard.html";
    else
        template_ = "../templates/login.html";
}

}
