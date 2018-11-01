#pragma once

#include "TemplateResource.h"

namespace Resources {

class ServiceResource : public TemplateResource
{
private:
    std::string id_;
protected:
    bool GetObjects(std::map<std::string, ginger::object>& objects) override final;
public:
    explicit ServiceResource(std::shared_ptr<HttpsServer::Request> request);
    void Render(std::shared_ptr<HttpsServer::Response> response) override;
};

}
