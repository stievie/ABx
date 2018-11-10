#pragma once

#include "Resource.h"

namespace Resources {

class AccountsJsonResource : public Resource
{
public:
    explicit AccountsJsonResource(std::shared_ptr<HttpsServer::Request> request) :
        Resource(request)
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override final;
};

}
