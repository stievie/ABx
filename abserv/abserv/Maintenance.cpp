#include "stdafx.h"
#include "Maintenance.h"
#include "Scheduler.h"
#include "DataProvider.h"
#include "GameManager.h"
#include "PlayerManager.h"
#include "Logger.h"
#include "CpuUsage.h"
#include "Application.h"
#include <AB/Entities/Service.h>
#include "ConfigManager.h"
#include "DataClient.h"
#include "Chat.h"
#include "Dispatcher.h"

#include "DebugNew.h"

Maintenance Maintenance::Instance;

void Maintenance::CleanCacheTask()
{
    IO::DataProvider::Instance.CleanCache();
    if (status_ == MaintenanceStatus::Runnig)
    {
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(CLEAN_CACHE_MS, std::bind(&Maintenance::CleanCacheTask, this))
        );
    }
}

void Maintenance::CleanGamesTask()
{
    Game::GameManager::Instance.CleanGames();
    if (status_ == MaintenanceStatus::Runnig)
    {
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(CLEAN_GAMES_MS, std::bind(&Maintenance::CleanGamesTask, this))
        );
    }
}

void Maintenance::CleanPlayersTask()
{
    Game::PlayerManager::Instance.CleanPlayers();
    if (status_ == MaintenanceStatus::Runnig)
    {
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(CLEAN_PLAYERS_MS, std::bind(&Maintenance::CleanPlayersTask, this))
        );
    }
}

void Maintenance::CleanChatsTask()
{
    Game::Chat::Instance.CleanChats();
    if (status_ == MaintenanceStatus::Runnig)
    {
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(CLEAN_CHATS_MS, std::bind(&Maintenance::CleanChatsTask, this))
        );
    }
}

void Maintenance::UpdateServerLoadTask()
{
    if (status_ != MaintenanceStatus::Runnig)
        return;

    AB::Entities::Service serv;
    serv.uuid = Application::Instance->GetServerId();
    IO::DataClient* cli = Application::Instance->GetDataClient();
    if (cli->Read(serv))
    {
        uint8_t load = Application::Instance->GetLoad();
        if (load != serv.load)
        {
            serv.load = load;
            cli->Update(serv);
        }
    }

    Asynch::Scheduler::Instance.Add(
        Asynch::CreateScheduledTask(UPDATE_SERVER_LOAD_MS, std::bind(&Maintenance::UpdateServerLoadTask, this))
    );
}

void Maintenance::CheckAutoTerminate()
{
    if (Game::PlayerManager::Instance.GetPlayerCount() == 0)
    {
        Asynch::Dispatcher::Instance.Add(Asynch::CreateTask(std::bind(&Application::Stop, Application::Instance)));
        return;
    }
    Asynch::Scheduler::Instance.Add(
        Asynch::CreateScheduledTask(CHECK_AUTOTERMINATE_MS, std::bind(&Maintenance::CheckAutoTerminate, this))
    );
}

void Maintenance::Run()
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        status_ = MaintenanceStatus::Runnig;
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
        Asynch::CreateScheduledTask(UPDATE_SERVER_LOAD_MS, std::bind(&Maintenance::UpdateServerLoadTask, this))
    );
    if (Application::Instance->autoTerminate_)
    {
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(CHECK_AUTOTERMINATE_MS, std::bind(&Maintenance::CheckAutoTerminate, this))
        );
    }
}

void Maintenance::Stop()
{
    std::lock_guard<std::mutex> lock(lock_);
    status_ = MaintenanceStatus::Runnig;
}
