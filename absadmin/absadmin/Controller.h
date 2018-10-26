#pragma once

#include "Cookie.h"
#include "Variant.h"

class Controller
{
protected:
    std::unique_ptr<Cookies> requestCookies_;
    std::unique_ptr<Cookies> responseCookies_;
    std::weak_ptr<Utils::VariantMap> session_;
public:
    Controller() = default;
    Controller(const Controller&) = delete;
    Controller& operator=(const Controller&) = delete;
    virtual ~Controller() = default;

    virtual void MakeRequest(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request);
};

class GetController : public Controller
{
};

class PostController : public Controller
{
};
