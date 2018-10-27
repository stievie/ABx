#include "stdafx.h"
#include "ServicesResource.h"
#include "StringHash.h"
#include "Application.h"

namespace Resources {

bool ServicesResource::GetObjects(std::map<std::string, ginger::object>& objects)
{
    objects["title"] = "ABS Admin";
    auto it = session_->values_.find(Utils::StringHashRt("username"));
    if (it != session_->values_.end())
        objects["user"] = (*it).second.GetString();
    else
        objects["user"] = "";
    return true;
}

std::string ServicesResource::GetTemplate()
{
    return "../templates/services.html";
}

void ServicesResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    bool loggedIn = session_->values_[Utils::StringHashRt("logged_in")].GetBool();
    if (loggedIn)
        TemplateResource::Render(response);
    else
        Redirect(response, "/");
}

}
