#pragma once

#include <AB/Entities/Ban.h>

namespace DB {

class DBBan
{
public:
    DBBan() = delete;
    ~DBBan() = delete;

    static bool Create(AB::Entities::Ban& ban);
    static bool Load(AB::Entities::Ban& ban);
    static bool Save(const AB::Entities::Ban& ban);
    static bool Delete(const AB::Entities::Ban& ban);
    static bool Exists(const AB::Entities::Ban& ban);
};

}
