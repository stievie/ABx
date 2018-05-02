#include "stdafx.h"
#include "Maintenance.h"
#include "Scheduler.h"
#include "DataProvider.h"
#include "GameManager.h"
#include "PlayerManager.h"
#include "Logger.h"

#include "DebugNew.h"

Maintenance Maintenance::Instance;

void Maintenance::CleanCacheTask()
{
    IO::DataProvider::Instance.CleanCache();
    if (status_ == StatusRunnig)
    {
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(CLEAN_CACHE_MS, std::bind(&Maintenance::CleanCacheTask, this))
        );
    }
}

void Maintenance::CleanGamesTask()
{
    Game::GameManager::Instance.CleanGames();
    if (status_ == StatusRunnig)
    {
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(CLEAN_GAMES_MS, std::bind(&Maintenance::CleanGamesTask, this))
        );
    }
}

void Maintenance::CleanPlayersTask()
{
    Game::PlayerManager::Instance.CleanPlayers();
    if (status_ == StatusRunnig)
    {
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(CLEAN_PLAYERS_MS, std::bind(&Maintenance::CleanPlayersTask, this))
        );
    }
}

void Maintenance::LogRotateTask()
{
    if (IO::Logger::Instance().logDir_.empty())
        return;
    IO::Logger::Instance().Close();
    if (status_ == StatusRunnig)
    {
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(LOG_ROTATE_INTERVAL, std::bind(&Maintenance::LogRotateTask, this))
        );
    }
}

void Maintenance::Run()
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        status_ = StatusRunnig;
    }
    Asynch::Scheduler::Instance.Add(
        Asynch::CreateScheduledTask(CLEAN_CACHE_MS, std::bind(&Maintenance::CleanCacheTask, this))
    );
    Asynch::Scheduler::Instance.Add(
        Asynch::CreateScheduledTask(CLEAN_GAMES_MS, std::bind(&Maintenance::CleanGamesTask, this))
    );
    Asynch::Scheduler::Instance.Add(
        Asynch::CreateScheduledTask(CLEAN_PLAYERS_MS, std::bind(&Maintenance::CleanPlayersTask, this))
    );
    Asynch::Scheduler::Instance.Add(
        Asynch::CreateScheduledTask(LOG_ROTATE_INTERVAL, std::bind(&Maintenance::LogRotateTask, this))
    );
}

void Maintenance::Stop()
{
    std::lock_guard<std::mutex> lock(lock_);
    status_ = StatusTerminated;
}
