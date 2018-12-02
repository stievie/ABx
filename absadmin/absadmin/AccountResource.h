#pragma once

#include "TemplateResource.h"

namespace Resources {

class AccountResource : public TemplateResource
{
private:
    std::string id_;
protected:
    bool GetObjects(std::map<std::string, ginger::object>& objects) override final;
public:
    explicit AccountResource(std::shared_ptr<HttpsServer::Request> request);
    void Render(std::shared_ptr<HttpsServer::Response> response) override;
};

}
