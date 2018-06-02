#pragma once

#include <AB/Entities/Guild.h>

namespace DB {

class DBGuild
{
public:
    DBGuild() = delete;
    ~DBGuild() = delete;

    static bool Create(AB::Entities::Guild& g);
    static bool Load(AB::Entities::Guild& g);
    static bool Save(const AB::Entities::Guild& g);
    static bool Delete(const AB::Entities::Guild& g);
    static bool Exists(const AB::Entities::Guild& g);
};

}
