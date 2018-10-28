#pragma once

#include "TemplateResource.h"

namespace Resources {

class IndexResource : public TemplateResource
{
protected:
    bool GetObjects(std::map<std::string, ginger::object>& objects) override final;
    std::string GetTemplate() override final;
public:
    explicit IndexResource(std::shared_ptr<HttpsServer::Request> request);
};

}
