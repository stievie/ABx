#pragma once

#include "Service.h"
#include "ServerApp.h"
#include "MessageClient.h"
#include "MessageDispatcher.h"
#include "Maintenance.h"
#include "Subsystems.h"

class Application : public ServerApp
{
private:
    asio::io_service ioService_;
    std::mutex loaderLock_;
    std::condition_variable loaderSignal_;
    std::unique_ptr<Net::ServiceManager> serviceManager_;
    std::unique_ptr<MessageDispatcher> msgDispatcher_;
    std::vector<int> loads_;
    int64_t lastLoadCalc_;
    Maintenance maintenance_;
    bool LoadMain();
    void PrintServerInfo();
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
protected:
    bool ParseCommandLine() override;
public:
    Application();
    ~Application() override;

    bool Initialize(const std::vector<std::string>& args) override;
    void Run();
    void Stop();

    std::string GetKeysFile() const;
    /// Returns a value between 0..100
    uint8_t GetLoad();

    void SpawnServer();

    bool autoTerminate_;
    bool temporary_;

    static Application* Instance;
};
