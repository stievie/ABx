#pragma once

#include "FileResource.h"
#include <mutex>

namespace Resources {

class LessFileResource : public FileResource
{
private:
    std::mutex mutex_;
    void CompileFile(const std::string& source, const std::string& dest);
public:
    explicit LessFileResource(std::shared_ptr<HttpsServer::Request> request) :
        FileResource(request),
        mutex_()
    { }
    void Render(std::shared_ptr<HttpsServer::Response> response) override;
};

}
