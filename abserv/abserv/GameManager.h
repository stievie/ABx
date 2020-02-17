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

#pragma once

#include "Game.h"
#include <limits>
#include <abscommon/Service.h>
#include <sa/IdGenerator.h>
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
    sa::IdGenerator<uint32_t> gameIds_;
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
    bool InstanceExists(const std::string& uuid);
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
