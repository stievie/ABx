#pragma once

#include <thread>
#include <mutex>
#include "Service.h"
#include <vector>
#include "Game.h"
#include <stdint.h>
#include <limits>
#pragma warning(push)
#pragma warning(disable: 4702 4127 4244)
#include <kaguya/kaguya.hpp>
#pragma warning(pop)

namespace Game {

class Player;

class GameManager
{
public:
    enum State
    {
        Running,
        Terminated
    };
private:
    static void LuaErrorHandler(int errCode, const char* message);
    Net::ServiceManager* serviceManager_;
    State state_;
    std::recursive_mutex lock_;
    std::map<uint32_t, std::shared_ptr<Game>> games_;
    std::map<std::string, std::vector<Game*>> maps_;
    uint32_t gameIds_ = 0;
    uint32_t GetNewGameId()
    {
        if (gameIds_ >= std::numeric_limits<uint32_t>::max())
            gameIds_ = 0;
        return ++gameIds_;
    }
    std::shared_ptr<Game> CreateGame(const std::string& mapName);
protected:
    friend class Game;
    void DeleteGameTask(uint32_t gameId);
public:
    GameManager() :
        state_(Terminated)
    { }

    void Start(Net::ServiceManager* serviceManager);
    void Stop();
    /// Returns the game with the mapName. If no such game exists it creates one.
    std::shared_ptr<Game> GetGame(const std::string& mapName, bool canCreate = false);
    bool AddPlayer(const std::string& mapName, std::shared_ptr<Player> player);
    // Delete all games with no players
    void CleanGames();

    GameManager::State GetState() const { return state_; }

    static void RegisterLuaAll(kaguya::State& state);
public:
    static GameManager Instance;
};

}
