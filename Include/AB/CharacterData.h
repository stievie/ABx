#pragma once

#include <vector>
#include <AB/CreatureData.h>

namespace AB {
namespace Data {

struct CharacterData : public CreatureData
{
    /// PvP only character
    bool pvp;
    uint64_t xp;
    uint32_t skillPoints;
    std::string lastMap;
};

typedef std::vector<CharacterData> CharacterList;

}
}
