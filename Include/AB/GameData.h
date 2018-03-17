#pragma once

#include <vector>

namespace AB {
namespace Data {

struct GameData
{
    uint32_t id;
    std::string name;
    uint8_t type;
};

typedef std::vector<GameData> GameList;

}
}
