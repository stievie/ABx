#pragma once

#include <AB/Entities/FriendedMe.h>

namespace DB {

class DBFriendedMe
{
public:
    DBFriendedMe() = delete;
    ~DBFriendedMe() = delete;

    static bool Create(AB::Entities::FriendedMe& fl);
    static bool Load(AB::Entities::FriendedMe& fl);
    static bool Save(const AB::Entities::FriendedMe& fl);
    static bool Delete(const AB::Entities::FriendedMe& fl);
    static bool Exists(const AB::Entities::FriendedMe& fl);
};

}
