#pragma once

#include <AB/Entities/Service.h>

namespace DB {

class DBService
{
public:
    DBService() = delete;
    ~DBService() = delete;

    static bool Create(AB::Entities::Service& s);
    static bool Load(AB::Entities::Service& s);
    static bool Save(const AB::Entities::Service& s);
    static bool Delete(const AB::Entities::Service& s);
    static bool Exists(const AB::Entities::Service& s);
};

}
