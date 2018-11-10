#pragma once

#include "Resource.h"

namespace Resources {

class TemplateResource : public Resource
{
private:
    void LoadTemplates(std::string& result);
    std::string GetTemplateFile(const std::string& templ);
protected:
    std::vector<std::string> headerScripts_;
    std::vector<std::string> footerScripts_;
    std::vector<std::string> styles_;
    std::string template_;
    virtual bool GetObjects(std::map<std::string, ginger::object>& objects);
public:
    explicit TemplateResource(std::shared_ptr<HttpsServer::Request> request);
    void Render(std::shared_ptr<HttpsServer::Response> response) override;
};

}
