#include "stdafx.h"
#include "DownloadResource.h"
#include "StringHash.h"
#include "DataClient.h"
#include <AB/Entities/Account.h>
#include "Subsystems.h"
#include "Xml.h"

namespace Resources {

DownloadResource::DownloadResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request)
{
    template_ = "../templates/download.html";
}

bool DownloadResource::GetObjects(std::map<std::string, ginger::object>& objects)
{
    if (!TemplateResource::GetObjects(objects))
        return false;
    return true;
}

void DownloadResource::Render(std::shared_ptr<HttpsServer::Response> response)
{
    if (!IsAllowed(AB::Entities::AccountTypeNormal))
    {
        Redirect(response, "/");
        return;
    }

    TemplateResource::Render(response);
}

}
