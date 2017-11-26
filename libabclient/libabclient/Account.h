#pragma once

#include <stdint.h>
#include <vector>
#include <string>

struct AccountCharacter
{
    uint32_t id;
    uint16_t level;
    std::string name;
};

typedef std::vector<AccountCharacter> Charlist;
