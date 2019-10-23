#pragma once

#include "ServerApp.h"
#include <AB/Entities/Account.h>
#if __cplusplus < 201703L
#   if !defined(__clang__) && !defined(__GNUC__)
#       include <filesystem>
#   else
#       include <experimental/filesystem>
#   endif
#else
#   include <filesystem>
#endif
#include <map>
#include "MessageClient.h"
#include "Servers.h"

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
    std::shared_ptr<asio::io_service> ioService_;
    std::unique_ptr<HttpsServer> server_;
    std::unique_ptr<HttpServer> httpServer_;
    void HtttpsRedirect(std::shared_ptr<HttpServer::Response> response,
        std::shared_ptr<HttpServer::Request> request);
    void InitContentTypes();
    void InitRoutes();
    void PrintServerInfo();
    void HandleMessage(const Net::MessageMsg& msg);
    void HandleError(std::shared_ptr<HttpsServer::Request> /*request*/,
        const SimpleWeb::error_code& ec);
    bool HandleOnAccept(const asio::ip::tcp::endpoint& endpoint);
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
    ~Application() override;

    bool Initialize(const std::vector<std::string>& args) override;
    void Run() override;
    void Stop() override;

    const std::string& GetRoot() const { return root_; }
    const std::string& GetHost() const { return serverHost_; }

    static SimpleWeb::CaseInsensitiveMultimap GetDefaultHeader();
    static Application* Instance;
};

