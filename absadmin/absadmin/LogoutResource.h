#pragma once

#include "Resource.h"

namespace Resources {

class LogoutResource : public Resource
{
public:
    explicit LogoutResource(std::shared_ptr<HttpsServer::Request> request) :
        Resource(request)
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override final;
};

}
