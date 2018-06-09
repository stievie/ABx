#pragma once

#include "Service.h"
#include "DataClient.h"
#include "ServerApp.h"
#include "MessageClient.h"
#include "MessageDispatcher.h"

class Application : public ServerApp
{
private:
    asio::io_service ioService_;
    std::mutex loaderLock_;
    std::condition_variable loaderSignal_;
    std::unique_ptr<Net::ServiceManager> serviceManager_;
    std::unique_ptr<IO::DataClient> dataClient_;
    std::unique_ptr<Net::MessageClient> msgClient_;
    std::unique_ptr<MessageDispatcher> msgDispatcher_;
    std::string configFile_;
    std::string logDir_;
    std::vector<int> loads_;
    int64_t lastLoadCalc_;
    std::string gameIp_;
    std::string gameHost_;
    uint16_t gamePort_;
    bool running_;
    bool LoadMain();
    void PrintServerInfo();
    bool ParseCommandLine();
    void ShowHelp();
    void HandleMessage(const Net::MessageMsg& msg);
    uint8_t GetAvgLoad() const
    {
        if (loads_.empty())
            return 0;

        float loads = 0.0f;
        for (int p : loads_)
            loads += static_cast<float>(p);
        return static_cast<uint8_t>(loads / loads_.size());
    }

public:
    Application();
    ~Application();

    bool Initialize(int argc, char** argv);
    void Run();
    void Stop();

    IO::DataClient* GetDataClient()
    {
        return dataClient_.get();
    }
    Net::MessageClient* GetMessageClient()
    {
        return msgClient_.get();
    }
    /// Returns a value between 0..100
    uint8_t GetLoad();
    /// Returns the Server UUID from the config file
    static const std::string& GetServerId();

    static Application* Instance;
};
