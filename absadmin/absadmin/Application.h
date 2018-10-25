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
#include "DataClient.h"

#if __cplusplus < 201703L
// C++14
namespace fs = std::experimental::filesystem;
#else
// C++17
namespace fs = std::filesystem;
#endif

using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;

class Application : public ServerApp, public std::enable_shared_from_this<Application>
{
private:
    bool running_;
    int64_t startTime_;
    std::string configFile_;
    std::string logDir_;
    std::string serverId_;
    std::string root_;
    std::string dataHost_;
    uint16_t dataPort_;
    uint16_t adminPort_;
    std::string adminIp_;
    std::string adminHost_;
    asio::io_service ioService_;
    std::unique_ptr<HttpsServer> server_;
    std::unique_ptr<IO::DataClient> dataClient_;
public:
    Application();
    ~Application();

    bool Initialize(int argc, char** argv) override;
    void Run() override;
    void Stop() override;
};

