#pragma once

#include <cereal/types/unordered_map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>
#include "Entity.h"

namespace Entities {

enum AccountType : uint8_t
{
    AccountTypeNormal = 1,
    AccountTypeTutor = 2,
    AccountTypeSeniorTutor = 3,
    AccountTypeGamemaster = 4,
    AccountTypeGod = 5
};

struct Account : Entity
{
    std::string name;
    std::string key;
    AccountType type;

    template <class Archive>
    void Save(Archive& ar) const
    {
        ar(id, name, key, type);
    }

    template <class Archive>
    void Load(Archive& ar)
    {
        ar(id, name, key, type);
    }
};

}