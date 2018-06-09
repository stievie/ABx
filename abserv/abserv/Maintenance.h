#pragma once

class Maintenance
{
private:
    std::mutex lock_;
    enum class MaintenanceStatus
    {
        Runnig,
        Terminated
    };
    MaintenanceStatus status_;
    void CleanCacheTask();
    void CleanGamesTask();
    void CleanPlayersTask();
    void CleanChatsTask();
    void UpdateServerLoadTask();
public:
    Maintenance() :
        status_(MaintenanceStatus::Terminated)
    {}
    ~Maintenance() = default;

    void Run();
    void Stop();

    static Maintenance Instance;
};

