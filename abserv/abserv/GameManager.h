#pragma once

#include "Service.h"
#include "Game.h"
#include <limits>
#include "IdGenerator.h"

namespace Game {

class Player;

class GameManager
{
public:
    enum State
    {
        ManagerStateRunning,
        ManagerStateTerminated
    };
private:
    static void LuaErrorHandler(int errCode, const char* message);
    State state_;
    std::recursive_mutex lock_;
    std::map<uint32_t, std::shared_ptr<Game>> games_;
    std::map<std::string, std::vector<Game*>> maps_;
    Utils::IdGenerator<uint32_t> gameIds_;
    uint32_t GetNewGameId()
    {
        return gameIds_.Next();
    }
    std::shared_ptr<Game> CreateGame(const std::string& mapUuid);
protected:
    friend class Game;
    void DeleteGameTask(uint32_t gameId);
public:
    GameManager() :
        state_(ManagerStateTerminated)
    { }

    void Start();
    void Stop();
    /// Creates a new game instance, e.g. when a group of players enters an instance
    std::shared_ptr<Game> NewGame(const std::string& mapName)
    {
        return CreateGame(mapName);
    }
    /// Returns the game with the mapName. If no such game exists it creates one.
    std::shared_ptr<Game> GetGame(const std::string& mapName, bool canCreate = false);
    std::shared_ptr<Game> Get(uint32_t gameId);
    bool AddPlayer(const std::string& mapUuid, std::shared_ptr<Player> player);
    // Delete all games with no players
    void CleanGames();

    GameManager::State GetState() const { return state_; }

    static void RegisterLuaAll(kaguya::State& state);
public:
    static GameManager Instance;
};

}
