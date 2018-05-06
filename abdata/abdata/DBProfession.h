#pragma once

#include <AB/Entities/Profession.h>

namespace DB {

class DBProfession
{
public:
    DBProfession() = delete;
    ~DBProfession() = delete;

    static bool Create(AB::Entities::Profession& prof);
    static bool Load(AB::Entities::Profession& prof);
    static bool Save(const AB::Entities::Profession& prof);
    static bool Delete(const AB::Entities::Profession& prof);
    static bool Exists(const AB::Entities::Profession& prof);
};

}
