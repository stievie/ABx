#include "stdafx.h"
#include "GameManager.h"
#include "Utils.h"
#include <algorithm>

#include "DebugNew.h"

namespace Game {

GameManager GameManager::Instance;

void GameManager::UpdateThread()
{
    int64_t prevTime = Utils::AbTick();
    while (state_ == Running)
    {
        int64_t start = Utils::AbTick();
        uint32_t delta = static_cast<uint32_t>(start - prevTime);
        prevTime = start;
        for (const auto& g : games_)
        {
            g->Update(delta);
        }
        int64_t end = Utils::AbTick();
        uint32_t duration = static_cast<uint32_t>(end - start);
        int32_t sleepTime = NETWORK_TICK - duration;
        if (sleepTime < 0)
            sleepTime = 5;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
    }
}

void GameManager::Start(Net::ServiceManager* serviceManager)
{
    serviceManager_ = serviceManager;
    state_ = State::Running;
    thread_ = std::thread(&GameManager::UpdateThread, this);
}

void GameManager::Stop()
{
    if (state_ == State::Running)
    {
        lock_.lock();
        state_ = Terminated;
        lock_.unlock();
        thread_.join();
    }
}

}
