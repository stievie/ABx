#pragma once

#include "Controller.h"

class GetFileController : public GetController
{
public:
    void MakeRequest(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request) override final;
};

