#include "stdafx.h"
#include "FriendsResource.h"
#include <AB/Entities/Account.h>

namespace Resources {

bool FriendsResource::GetObjects(std::map<std::string, ginger::object>& objects)
{
    // TODO: Implement this
    (void)objects;
    return false;
}

FriendsResource::FriendsResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request)
{
}

void FriendsResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountTypeNormal))
    {
        Redirect(response, "/");
        return;
    }

    TemplateResource::Render(response);
}
}
