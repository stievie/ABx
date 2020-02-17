/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include "Maintenance.h"
#include "AiDebugServer.h"
#include "Application.h"
#include "Chat.h"
#include "ConfigManager.h"
#include "DataProvider.h"
#include "GameManager.h"
#include "PlayerManager.h"
#include <AB/Entities/Service.h>
#include <abscommon/CpuUsage.h>
#include <abscommon/ThreadPool.h>

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
    auto* playerMan = GetSubsystem<Game::PlayerManager>();
    playerMan->CleanPlayers();
    playerMan->RefreshAuthTokens();
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

#ifdef DEBUG_POOLALLOCATOR
    // Print some stats
    sa::PoolInfo ninfo = Net::NetworkMessage::GetPoolInfo();
    LOG_DEBUG << "NetworkMessage Pool:  allocs: " << ninfo.allocs << ", frees: " << ninfo.frees << ", current: " << ninfo.current <<
        ", peak: " << ninfo.peak <<
        ", used: " << ninfo.used << ", avail: " << ninfo.avail << std::endl;
    sa::PoolInfo oinfo = Net::OutputMessagePool::GetPoolInfo();
    LOG_DEBUG << "OutputMessage Pool:  allocs: " << oinfo.allocs << ", frees: " << oinfo.frees << ", current: " << oinfo.current <<
        ", peak: " << oinfo.peak <<
        ", used: " << oinfo.used << ", avail: " << oinfo.avail << std::endl;
#endif

    AB::Entities::Service serv;
    serv.uuid = Application::Instance->GetServerId();
    IO::DataClient* cli = GetSubsystem<IO::DataClient>();
    if (cli->Read(serv))
    {
        uint8_t load = static_cast<uint8_t>(Application::Instance->GetLoad());
        if (load != serv.load || Utils::TimeElapsed(serv.heartbeat) >= AB::Entities::HEARTBEAT_INTERVAL)
        {
            serv.load = load;
            serv.heartbeat = Utils::Tick();
            cli->Update(serv);
        }
    }

    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(UPDATE_SERVER_LOAD_MS, std::bind(&Maintenance::UpdateServerLoadTask, this))
    );
}

void Maintenance::CheckAutoTerminate()
{
    if (GetSubsystem<Game::PlayerManager>()->GetIdleTime() >= CHECK_AUTOTERMINATE_IDLE_MS)
    {
        // Can't use Dispatcher because Stop() must run in a different thread. Stop() will wait
        // until all games are deleted, and games are deleted in the Dispatcher thread.
        GetSubsystem<Asynch::ThreadPool>()->Enqueue(&Application::Stop, Application::Instance);
        return;
    }
    if (status_ == MaintenanceStatus::Runnig)
    {
        GetSubsystem<Asynch::Dispatcher>()->Add(
            Asynch::CreateScheduledTask(CHECK_AUTOTERMINATE_MS, std::bind(&Maintenance::CheckAutoTerminate, this))
        );
    }
}

void Maintenance::UpdateAiServer()
{
    GetSubsystem<AI::DebugServer>()->Update();
    if (status_ == MaintenanceStatus::Runnig)
    {
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(aiUpdateInterval_, std::bind(&Maintenance::UpdateAiServer, this))
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
    if (GetSubsystem<AI::DebugServer>()->IsActive())
    {
        shed->Add(
            Asynch::CreateScheduledTask(AI_SERVER_UPDATE_INTERVAL, std::bind(&Maintenance::UpdateAiServer, this))
        );
    }
}

void Maintenance::Stop()
{
    std::lock_guard<std::mutex> lock(lock_);
    status_ = MaintenanceStatus::Runnig;
}
