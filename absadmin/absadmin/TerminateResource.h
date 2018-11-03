#pragma once

#include "Resource.h"

namespace Resources {

class TerminateResource : public Resource
{
private:
    bool Terminate(const std::string& uuid);
public:
    explicit TerminateResource(std::shared_ptr<HttpsServer::Request> request) :
        Resource(request)
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override final;
};

}