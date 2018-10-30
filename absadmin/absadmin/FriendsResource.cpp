#include "stdafx.h"
#include "FriendsResource.h"
#include "StringHash.h"

namespace Resources {

bool FriendsResource::GetObjects(std::map<std::string, ginger::object>& objects)
{
    return false;
}

FriendsResource::FriendsResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request)
{
}

void FriendsResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    bool loggedIn = session_->values_[Utils::StringHashRt("logged_in")].GetBool();
    if (loggedIn)
    {
    }
    else
        Redirect(response, "/");
}
}
