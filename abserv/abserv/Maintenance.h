#pragma once

class Maintenance
{
private:
    std::mutex lock_;
    enum MaintenanceStatus
    {
        StatusRunnig,
        StatusTerminated
    };
    MaintenanceStatus status_;
    void CleanCacheTask();
    void CleanGamesTask();
    void CleanPlayersTask();
    void LogRotateTask();
public:
    Maintenance() :
        status_(StatusTerminated)
    {}
    ~Maintenance() = default;

    void Run();
    void Stop();

    static Maintenance Instance;
};

