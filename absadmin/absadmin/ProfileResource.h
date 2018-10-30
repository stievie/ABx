#pragma once

#include "TemplateResource.h"

namespace Resources {

class ProfileResource : public TemplateResource
{
protected:
    bool GetObjects(std::map<std::string, ginger::object>& objects) override final;
public:
    explicit ProfileResource(std::shared_ptr<HttpsServer::Request> request);
    void Render(std::shared_ptr<HttpsServer::Response> response) override;
};

}
