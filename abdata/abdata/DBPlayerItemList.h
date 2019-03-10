#pragma once

#include <AB/Entities/PlayerItemList.h>

namespace DB {

class DBPlayerItemList
{
public:
    DBPlayerItemList() = delete;
    ~DBPlayerItemList() = delete;

    static bool Create(AB::Entities::PlayerItemList& il);
    static bool Load(AB::Entities::PlayerItemList& il);
    static bool Save(const AB::Entities::PlayerItemList& il);
    static bool Delete(const AB::Entities::PlayerItemList& il);
    static bool Exists(const AB::Entities::PlayerItemList& il);
};

}
