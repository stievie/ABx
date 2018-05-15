#pragma once

#include "Player.h"
#include <AB/Entities/Character.h>

namespace IO {

class IOPlayer
{
private:
    static bool LoadPlayer(Game::Player* player);
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

    static bool LoadCharacter(AB::Entities::Character& ch);
    static bool LoadPlayerByName(Game::Player* player, const std::string& name);
    static bool LoadPlayerByUuid(Game::Player* player, const std::string& uuid);
    static bool SavePlayer(Game::Player* player);
    static IOPlayer::CreatePlayerResult CreatePlayer(const std::string& accountUuid,
        const std::string& name, const std::string& profUuid, AB::Entities::CharacterSex sex, bool isPvp,
        std::string& uuid);
    static bool DeletePlayer(const std::string& accountUuid, const std::string& playerUuid);
};

}
