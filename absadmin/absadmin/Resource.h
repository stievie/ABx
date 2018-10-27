#pragma once

#include "Cookie.h"
#include "Sessions.h"

namespace Resources {

class Resource
{
protected:
    std::shared_ptr<HttpsServer::Request> request_;
    std::unique_ptr<HTTP::Cookies> requestCookies_;
    std::unique_ptr<HTTP::Cookies> responseCookies_;
    std::shared_ptr<HTTP::Session> session_;
public:
    explicit Resource(std::shared_ptr<HttpsServer::Request> request);
    Resource() = default;
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    virtual ~Resource() = default;

    virtual void Render(std::shared_ptr<HttpsServer::Response> response) = 0;
};

}
