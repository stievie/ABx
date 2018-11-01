#pragma once

#include "Resource.h"

namespace Resources {

class SpawnResource : public Resource
{
private:
    bool Spawn(const std::string& uuid);
public:
    explicit SpawnResource(std::shared_ptr<HttpsServer::Request> request) :
        Resource(request)
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override final;
};

}
