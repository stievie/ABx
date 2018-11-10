#pragma once

#include "TemplateResource.h"

namespace Resources {

class AccountsResource : public TemplateResource
{
protected:
    bool GetObjects(std::map<std::string, ginger::object>& objects) override final;
public:
    explicit AccountsResource(std::shared_ptr<HttpsServer::Request> request);
    void Render(std::shared_ptr<HttpsServer::Response> response) override;
};

}
