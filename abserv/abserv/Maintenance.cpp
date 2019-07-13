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
#include "Subsystems.h"

#include "DebugNew.h"

void Maintenance::CleanCacheTask()
{
    GetSubsystem<IO::DataProvider>()->CleanCache();
    if (status_ == MaintenanceStatus::Runnig)
    {
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(CLEAN_CACHE_MS, std::bind(&Maintenance::CleanCacheTask, this))
        );
    }
}

void Maintenance::CleanGamesTask()
{
    GetSubsystem<Game::GameManager>()->CleanGames();
    if (status_ == MaintenanceStatus::Runnig)
    {
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(CLEAN_GAMES_MS, std::bind(&Maintenance::CleanGamesTask, this))
        );
    }
}

void Maintenance::CleanPlayersTask()
{
    GetSubsystem<Game::PlayerManager>()->CleanPlayers();
    if (status_ == MaintenanceStatus::Runnig)
    {
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(CLEAN_PLAYERS_MS, std::bind(&Maintenance::CleanPlayersTask, this))
        );
    }
}

void Maintenance::CleanChatsTask()
{
    GetSubsystem<Game::Chat>()->CleanChats();
    if (status_ == MaintenanceStatus::Runnig)
    {
        GetSubsystem<Asynch::Scheduler>()->Add(
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
    IO::DataClient* cli = GetSubsystem<IO::DataClient>();
    if (cli->Read(serv))
    {
        uint8_t load = static_cast<uint8_t>(Application::Instance->GetLoad());
        if (load != serv.load || Utils::TimeElapsed(serv.heardbeat) >= AB::Entities::HEARDBEAT_INTERVAL)
        {
            serv.load = load;
            serv.heardbeat = Utils::Tick();
            cli->Update(serv);
        }
    }

    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(UPDATE_SERVER_LOAD_MS, std::bind(&Maintenance::UpdateServerLoadTask, this))
    );
}

void Maintenance::CheckAutoTerminate()
{
    auto* dispatcher = GetSubsystem<Asynch::Dispatcher>();
    if (GetSubsystem<Game::PlayerManager>()->GetIdleTime() >= CHECK_AUTOTERMINATE_IDLE_MS)
    {
        dispatcher->Add(Asynch::CreateTask(std::bind(&Application::Stop, Application::Instance)));
        return;
    }
    if (status_ == MaintenanceStatus::Runnig)
    {
        dispatcher->Add(
            Asynch::CreateScheduledTask(CHECK_AUTOTERMINATE_MS, std::bind(&Maintenance::CheckAutoTerminate, this))
        );
    }
}

void Maintenance::Run()
{
    {
        std::lock_guard<std::mutex> lock(lock_);
        status_ = MaintenanceStatus::Runnig;
    }
    auto* shed = GetSubsystem<Asynch::Scheduler>();
    shed->Add(
        Asynch::CreateScheduledTask(CLEAN_CACHE_MS, std::bind(&Maintenance::CleanCacheTask, this))
    );
    shed->Add(
        Asynch::CreateScheduledTask(CLEAN_GAMES_MS, std::bind(&Maintenance::CleanGamesTask, this))
    );
    shed->Add(
        Asynch::CreateScheduledTask(CLEAN_PLAYERS_MS, std::bind(&Maintenance::CleanPlayersTask, this))
    );
    shed->Add(
        Asynch::CreateScheduledTask(UPDATE_SERVER_LOAD_MS, std::bind(&Maintenance::UpdateServerLoadTask, this))
    );
    if (Application::Instance->autoTerminate_)
    {
        shed->Add(
            Asynch::CreateScheduledTask(CHECK_AUTOTERMINATE_MS, std::bind(&Maintenance::CheckAutoTerminate, this))
        );
    }
}

void Maintenance::Stop()
{
    std::lock_guard<std::mutex> lock(lock_);
    status_ = MaintenanceStatus::Runnig;
}
