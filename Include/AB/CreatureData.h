#pragma once

namespace AB {
namespace Data {

enum CreatureSex
{
    CreatureSexUnknown = 0,
    CreatureSexFemale,
    CreatureSexMale
};

struct CreatureData
{
    uint32_t id;
    uint16_t level;
    std::string name;
    std::string prof;
    std::string prof2;
    CreatureSex sex;
};

}
}
