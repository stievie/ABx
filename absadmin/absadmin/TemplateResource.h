#pragma once

#include "Resource.h"

namespace Resources {

class TemplateResource : public Resource
{
protected:
    virtual bool GetObjects(std::map<std::string, ginger::object>& objects) = 0;
    virtual std::string GetTemplate() = 0;
public:
    explicit TemplateResource(std::shared_ptr<HttpsServer::Request> request) :
        Resource(request)
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override final;
};

}
