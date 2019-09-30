#pragma once

#include "Service.h"
#include "ServerApp.h"
#include "MessageClient.h"
#include "MessageDispatcher.h"
#include "Maintenance.h"
#include "Subsystems.h"
#if defined(SCENE_VIEWER)
#include "SceneViewer.h"
#endif

class Application : public ServerApp
{
private:
    asio::io_service ioService_;
    std::mutex lock_;
    std::condition_variable loaderSignal_;
    std::unique_ptr<Net::ServiceManager> serviceManager_;
    std::unique_ptr<MessageDispatcher> msgDispatcher_;
    std::vector<unsigned> loads_;
    int64_t lastLoadCalc_{ 0 };
    Maintenance maintenance_;
    bool aiServer_{ false };
    std::string aiServerIp_;
    uint16_t aiServerPort_{ 0 };
#if defined(SCENE_VIEWER)
    std::shared_ptr<Debug::SceneViewer> sceneViewer_;
#endif
    bool LoadMain();
    void PrintServerInfo();
    void ShowHelp();
    void HandleMessage(const Net::MessageMsg& msg);
    void HandleCreateInstanceMessage(const Net::MessageMsg& msg);
    unsigned GetAvgLoad() const
    {
        if (loads_.size() == 0)
            return 0;
        return std::accumulate(loads_.begin(), loads_.end(), 0u) / loads_.size();
    }
protected:
    bool ParseCommandLine() override;
public:
    Application();
    ~Application() override;

    bool Initialize(const std::vector<std::string>& args) override;
    void Run() override;
    void Stop() override;

    std::string GetKeysFile() const;
    /// Returns a value between 0..100
    unsigned GetLoad();

    void SpawnServer();

    bool autoTerminate_{ false };
    bool temporary_{ false };

    static Application* Instance;
};
