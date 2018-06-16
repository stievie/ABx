#pragma once

#include <AB/Entities/GuildMembers.h>

class StorageProvider;

namespace DB {

class DBGuildMembers
{
public:
    DBGuildMembers() = delete;
    ~DBGuildMembers() = delete;

    static bool Create(AB::Entities::GuildMembers& g);
    static bool Load(AB::Entities::GuildMembers& g);
    static bool Save(const AB::Entities::GuildMembers& g);
    static bool Delete(const AB::Entities::GuildMembers& g);
    static bool Exists(const AB::Entities::GuildMembers& g);
    /// Delete expired guild member from DB
    static void DeleteExpired(StorageProvider* sp);
};

}
