#pragma once

#include <vector>

namespace AB {
namespace Data {

enum GameType : uint8_t
{
    GameTypeUnknown = 0,
    GameTypeOutpost = 1,
    GameTypePvPCombat,
    GameTypeExploreable,
    GameTypeMission,
};

struct GameData
{
    uint32_t id;
    std::string name;
    GameType type;
};

typedef std::vector<GameData> GameList;

}
}
