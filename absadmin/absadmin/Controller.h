#pragma once

#include "Cookie.h"
#include "Variant.h"
#include "Sessions.h"

class Controller
{
protected:
    std::unique_ptr<HTTP::Cookies> requestCookies_;
    std::unique_ptr<HTTP::Cookies> responseCookies_;
    std::weak_ptr<HTTP::Session> session_;
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

