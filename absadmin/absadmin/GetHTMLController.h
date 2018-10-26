#pragma once

#include "Controller.h"

class GetHTMLController : public GetController
{
protected:
    virtual bool GetObjects(std::map<std::string, ginger::object>& objects);
public:
    void MakeRequest(std::shared_ptr<HttpsServer::Response> response,
        std::shared_ptr<HttpsServer::Request> request) override final;
};

