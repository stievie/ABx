#pragma once

#include "Service.h"
#include "Game.h"
#include <limits>
#include "IdGenerator.h"
#include <AB/Entities/Game.h>

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
    State state_;
    std::mutex lock_;
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
    std::shared_ptr<Game> GetInstance(const std::string& instanceUuid);
    std::shared_ptr<Game> GetOrCreateInstance(const std::string& mapUuid, const std::string& instanceUuid);
    /// Returns the game with the mapName. If no such game exists it creates one.
    std::shared_ptr<Game> GetGame(const std::string& mapName, bool canCreate = false);
    std::shared_ptr<Game> Get(uint32_t gameId);
    bool AddPlayer(const std::string& mapUuid, std::shared_ptr<Player> player);
    // Delete all games with no players
    void CleanGames();
    AB::Entities::GameType GetGameType(const std::string& mapUuid);

    GameManager::State GetState() const { return state_; }
    size_t GetGameCount() const { return games_.size(); }
    const std::map<uint32_t, std::shared_ptr<Game>>& GetGames() const { return games_; }
};

}
