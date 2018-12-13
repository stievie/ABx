#pragma once

#include <AB/Entities/Music.h>

namespace DB {

class DBMusic
{
public:
    DBMusic() = delete;
    ~DBMusic() = delete;

    static bool Create(AB::Entities::Music& item);
    static bool Load(AB::Entities::Music& item);
    static bool Save(const AB::Entities::Music& item);
    static bool Delete(const AB::Entities::Music& item);
    static bool Exists(const AB::Entities::Music& item);
};

}
