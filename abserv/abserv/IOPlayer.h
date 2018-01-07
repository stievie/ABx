#pragma once

#include "Player.h"
#include "Database.h"

namespace DB {

class IOPlayer
{
private:
    static bool LoadPlayer(Game::Player* player, std::shared_ptr<DBResult> result);
public:
    enum CreatePlayerResult
    {
        ResultOK,
        ResultNameExists,
        ResultInvalidAccount,
        ResultNoMoreCharSlots,
        ResultInternalError
    };
    IOPlayer() = delete;

    static bool PreloadPlayer(Game::Player* player, const std::string& name);
    static bool LoadPlayerByName(Game::Player* player, const std::string& name);
    static bool LoadPlayerById(Game::Player* player, uint32_t playerId);
    static bool SavePlayer(Game::Player* player);
    static IOPlayer::CreatePlayerResult CreatePlayer(uint32_t accountId,
        std::string& name, const std::string& prof, Game::PlayerSex sex, bool isPvp);
    static bool DeletePlayer(uint32_t accountId, uint32_t playerId);
};

}
