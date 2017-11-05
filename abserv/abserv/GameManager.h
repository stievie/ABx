#pragma once

#include <thread>
#include <mutex>
#include "Service.h"
#include <vector>
#include "Game.h"
#include <stdint.h>
#include <limits>

namespace Game {

class GameManager
{
public:
    enum State
    {
        Running,
        Terminated
    };
private:
    Net::ServiceManager* serviceManager_;
    std::thread thread_;
    State state_;
    std::mutex lock_;
    std::vector<std::shared_ptr<Game>> games_;
    uint32_t gameIds_ = 0;
    uint32_t GetNewGameId()
    {
        if (gameIds_ >= std::numeric_limits<uint32_t>::max())
            gameIds_ = 0;
        return gameIds_++;
    }
    void UpdateThread();
public:
    GameManager() :
        state_(Terminated)
    { }

    void Start(Net::ServiceManager* serviceManager);
    void Stop();

    GameManager::State GetState() const { return state_; }
public:
    static GameManager Instance;
};

}
