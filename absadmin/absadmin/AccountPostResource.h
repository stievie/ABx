#pragma once

#include "Resource.h"

namespace Resources {

class AccountPostResource : public Resource
{
public:
    explicit AccountPostResource(std::shared_ptr<HttpsServer::Request> request) :
        Resource(request)
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override final;
};

}
