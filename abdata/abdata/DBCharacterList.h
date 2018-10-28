#pragma once

#include <AB/Entities/CharacterList.h>

namespace DB {

class DBCharacterList
{
public:
    DBCharacterList() = delete;
    ~DBCharacterList() = delete;

    static bool Create(AB::Entities::CharacterList&);
    static bool Load(AB::Entities::CharacterList& al);
    static bool Save(const AB::Entities::CharacterList&);
    static bool Delete(const AB::Entities::CharacterList&);
    static bool Exists(const AB::Entities::CharacterList&);
};

}
