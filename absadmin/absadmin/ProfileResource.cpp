#include "stdafx.h"
#include "ProfileResource.h"
#include "StringHash.h"
#include "DataClient.h"
#include <AB/Entities/Account.h>
#include "Subsystems.h"

namespace Resources {

ProfileResource::ProfileResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request)
{
    template_ = "../templates/profile.html";

    // Bootstrap
    footerScripts_.push_back("vendors/bootstrap/dist/js/bootstrap.min.js");
    footerScripts_.push_back("js/common.js");
}

bool ProfileResource::GetObjects(std::map<std::string, ginger::object>& objects)
{
    if (!TemplateResource::GetObjects(objects))
        return false;

    std::string uuid = session_->values_[Utils::StringHashRt("account_uuid")].GetString();
    AB::Entities::Account account;
    account.uuid = uuid;
    auto dataClient = GetSubsystem<IO::DataClient>();
    if (!dataClient->Read(account))
        return false;

    std::map<std::string, ginger::object> xs;
    xs["uuid"] = account.uuid;
    xs["name"] = account.name;
    xs["email"] = account.email;

    objects["account"] = xs;
    return true;
}

void ProfileResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    bool loggedIn = session_->values_[Utils::StringHashRt("logged_in")].GetBool();
    if (loggedIn)
        TemplateResource::Render(response);
    else
        Redirect(response, "/");
}

}
