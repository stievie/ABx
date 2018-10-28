#pragma once

#include "Resource.h"

namespace Resources {

class ServicesJsonResource : public Resource
{
public:
    explicit ServicesJsonResource(std::shared_ptr<HttpsServer::Request> request) :
        Resource(request)
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override final;
};

}
