#pragma once

#include "Resource.h"

namespace Resources {

class AccountLogoutResource : public Resource
{
private:
    bool Logout(const std::string& uuid);
public:
    explicit AccountLogoutResource(std::shared_ptr<HttpsServer::Request> request) :
        Resource(request)
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override final;
};

}