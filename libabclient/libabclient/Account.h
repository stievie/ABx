#pragma once

#include <vector>

namespace Client {

struct AccountCharacter
{
    uint32_t id;
    uint16_t level;
    std::string name;
    std::string prof;
    std::string prof2;
    std::string lastMap;
};

typedef std::vector<AccountCharacter> CharList;

}
