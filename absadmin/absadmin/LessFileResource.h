#pragma once

#include "FileResource.h"

namespace Resources {

class LessFileResource : public FileResource
{
private:
    void CompileFile(const std::string& source, const std::string& dest);
public:
    explicit LessFileResource(std::shared_ptr<HttpsServer::Request> request) :
        FileResource(request)
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override;
};

}
