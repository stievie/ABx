#pragma once

#include "Resource.h"

namespace Resources {

class ProfilePostResource : public Resource
{
public:
    explicit ProfilePostResource(std::shared_ptr<HttpsServer::Request> request) :
        Resource(request)
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override final;
};

}
