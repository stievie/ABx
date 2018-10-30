#pragma once

#include "Resource.h"

namespace Resources {

class PasswordPostResource : public Resource
{
private:
    bool ChangePassword(const SimpleWeb::CaseInsensitiveMultimap& form);
public:
    explicit PasswordPostResource(std::shared_ptr<HttpsServer::Request> request) :
        Resource(request)
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override final;
};

}
