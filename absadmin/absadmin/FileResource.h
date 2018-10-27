#pragma once

#include "Resource.h"

namespace Resources {

class FileResource : public Resource
{
public:
    explicit FileResource(std::shared_ptr<HttpsServer::Request> request) :
        Resource(request)
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override final;
};

}
