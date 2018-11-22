#pragma once

#include "Resource.h"

namespace Resources {

class ClearCacheResource : public Resource
{
private:
    bool ClearCache(const std::string& uuid);
public:
    explicit ClearCacheResource(std::shared_ptr<HttpsServer::Request> request) :
        Resource(request)
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override final;
};

}
