#pragma once

#include "ServerApp.h"
#include <AB/Entities/Account.h>
#if __cplusplus < 201703L
#   if !defined(__clang__)
#       include <filesystem>
#   else
#       include <experimental/filesystem>
#   endif
#else
#   include <filesystem>
#endif
#include <map>
#include "MessageClient.h"

#if __cplusplus < 201703L
// C++14
namespace fs = std::experimental::filesystem;
#else
// C++17
namespace fs = std::filesystem;
#endif

class Application : public ServerApp, public std::enable_shared_from_this<Application>
{
private:
    int64_t startTime_;
    std::string root_;
    uint16_t adminPort_;
    std::string adminIp_;
    std::string adminHost_;
    asio::io_service ioService_;
    std::unique_ptr<HttpsServer> server_;
    bool ParseCommandLine();
    void ShowHelp();
    void PrintServerInfo();
    void HandleMessage(const Net::MessageMsg& msg);
    void HandleError(std::shared_ptr<HttpsServer::Request> /*request*/,
        const SimpleWeb::error_code& ec);
    template<typename C>
    void Route(const std::string& method, const std::string& pattern)
    {
        server_->resource[pattern][method] = [](std::shared_ptr<HttpsServer::Response> response,
            std::shared_ptr<HttpsServer::Request> request)
        {
            C c(request);
            c.Render(response);
        };
    }
    template<typename C>
    void DefaultRoute(const std::string& method)
    {
        server_->default_resource[method] = [](std::shared_ptr<HttpsServer::Response> response,
            std::shared_ptr<HttpsServer::Request> request)
        {
            C c(request);
            c.Render(response);
        };
    }
public:
    Application();
    ~Application();

    bool Initialize(int argc, char** argv) override;
    void Run() override;
    void Stop() override;

    const std::string& GetRoot() const { return root_; }
    const std::string& GetHost() const { return adminHost_; }

    static SimpleWeb::CaseInsensitiveMultimap GetDefaultHeader();
    static Application* Instance;
};

