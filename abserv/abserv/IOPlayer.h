#pragma once

#include "Player.h"
#include "Database.h"
#include <AB/Entities/Character.h>

namespace IO {

class IOPlayer
{
private:
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

    static bool LoadCharacter(AB::Entities::Character& player);
    static bool PreloadPlayer(Game::Player* player, const std::string& name);
    static bool LoadPlayerByName(Game::Player* player, const std::string& name);
    static bool SavePlayer(Game::Player* player);
    static IOPlayer::CreatePlayerResult CreatePlayer(const std::string& accountUuid,
        std::string& name, const std::string& prof, AB::Entities::CharacterSex sex, bool isPvp);
    static bool DeletePlayer(const std::string& accountUuid, const std::string& playerUuid);
};

}
