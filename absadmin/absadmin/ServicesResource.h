#pragma once

#include "TemplateResource.h"

namespace Resources {

class ServicesResource : public TemplateResource
{
protected:
    bool GetObjects(std::map<std::string, ginger::object>& objects) override final;
    std::string GetTemplate() override final;
public:
    explicit ServicesResource(std::shared_ptr<HttpsServer::Request> request);
    void Render(std::shared_ptr<HttpsServer::Response> response) override;
};

}
