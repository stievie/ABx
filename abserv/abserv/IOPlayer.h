#pragma once

#include "Player.h"
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
        ResultInvalidProfession,
        ResultInternalError
    };
    IOPlayer() = delete;

    static bool LoadCharacter(AB::Entities::Character& player);
    static bool PreloadPlayer(Game::Player* player, const std::string& name);
    static bool LoadPlayerByName(Game::Player* player, const std::string& name);
    static bool LoadPlayerByUuid(Game::Player* player, const std::string& uuid);
    static bool SavePlayer(Game::Player* player);
    static IOPlayer::CreatePlayerResult CreatePlayer(const std::string& accountUuid,
        const std::string& name, const std::string& prof, AB::Entities::CharacterSex sex, bool isPvp,
        std::string& uuid);
    static bool DeletePlayer(const std::string& accountUuid, const std::string& playerUuid);
};

}
