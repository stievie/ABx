#pragma once

#include "Cookie.h"
#include "Sessions.h"
#include <AB/Entities/Account.h>

namespace Resources {

class Resource
{
protected:
    std::shared_ptr<HttpsServer::Request> request_;
    std::unique_ptr<HTTP::Cookies> requestCookies_;
    std::unique_ptr<HTTP::Cookies> responseCookies_;
    std::shared_ptr<HTTP::Session> session_;
    void Redirect(std::shared_ptr<HttpsServer::Response> response, const std::string& url);
    bool IsAllowed(AB::Entities::AccountType minType);
public:
    explicit Resource(std::shared_ptr<HttpsServer::Request> request);
    Resource() = default;
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    virtual ~Resource() = default;

    virtual void Render(std::shared_ptr<HttpsServer::Response> response) = 0;
};

}
