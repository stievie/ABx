#pragma once

#include "Resource.h"

namespace Resources {

class UpdateAccountKeyResource: public Resource
{
private:
    bool Update(const std::string& uuid, const std::string& filed, const std::string& value);
public:
    explicit UpdateAccountKeyResource(std::shared_ptr<HttpsServer::Request> request) :
        Resource(request)
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override final;
};

}
