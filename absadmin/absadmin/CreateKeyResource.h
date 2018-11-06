#pragma once

#include "Resource.h"

namespace Resources {

class CreateKeyResource : public Resource
{
private:
    bool CreateKey(const std::string& uuid, const std::string& keyType,
        const std::string& count, const std::string& keyStatus,
        const std::string& email, const std::string& descr);
public:
    explicit CreateKeyResource(std::shared_ptr<HttpsServer::Request> request) :
        Resource(request)
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override final;
};

}