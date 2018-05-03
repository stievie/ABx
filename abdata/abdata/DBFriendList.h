#pragma once

#include <AB/Entities/FriendList.h>

namespace DB {

class DBFriendList
{
public:
    DBFriendList() = delete;
    ~DBFriendList() = delete;

    static bool Create(AB::Entities::FriendList& fl);
    static bool Load(AB::Entities::FriendList& fl);
    static bool Save(const AB::Entities::FriendList& fl);
    static bool Delete(const AB::Entities::FriendList& fl);
    static bool Exists(const AB::Entities::FriendList& fl);
};

}
