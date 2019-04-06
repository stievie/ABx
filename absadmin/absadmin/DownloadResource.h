#pragma once

#include "TemplateResource.h"

namespace Resources {

class DownloadResource : public TemplateResource
{
protected:
    bool GetObjects(std::map<std::string, ginger::object>& objects) override final;
public:
    explicit DownloadResource(std::shared_ptr<HttpsServer::Request> request);
    void Render(std::shared_ptr<HttpsServer::Response> response) override;
};

}
