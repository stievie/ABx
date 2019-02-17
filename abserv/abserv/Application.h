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
    std::mutex loaderLock_;
    std::condition_variable loaderSignal_;
    std::unique_ptr<Net::ServiceManager> serviceManager_;
    std::unique_ptr<MessageDispatcher> msgDispatcher_;
    std::vector<unsigned> loads_;
    int64_t lastLoadCalc_;
    Maintenance maintenance_;
#if defined(SCENE_VIEWER)
    std::shared_ptr<Debug::SceneViewer> sceneViewer_;
#endif
    bool LoadMain();
    void PrintServerInfo();
    void ShowHelp();
    void HandleMessage(const Net::MessageMsg& msg);
    unsigned GetAvgLoad() const
    {
        if (loads_.empty())
            return 0;

        float loads = 0.0f;
        for (int p : loads_)
            loads += static_cast<float>(p);
        return static_cast<unsigned>(loads / loads_.size());
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
    unsigned GetLoad();

    void SpawnServer();

    bool autoTerminate_;
    bool temporary_;

    static Application* Instance;
};
