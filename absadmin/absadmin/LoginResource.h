#pragma once

#include "Resource.h"

namespace Resources {

class LoginResource : public Resource
{
private:
    bool Auth(const std::string& user, const std::string& pass);
public:
    explicit LoginResource(std::shared_ptr<HttpsServer::Request> request) :
        Resource(request)
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override final;
};

}
