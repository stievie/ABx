#pragma once

#include <vector>

namespace Client
{

struct Game
{
    uint32_t id;
    std::string name;
    uint8_t type;
};

typedef std::vector<Game> GameList;

}