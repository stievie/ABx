#pragma once

#include <AB/Entities/GameInstance.h>

namespace DB {

class DBInstance
{
public:
    DBInstance() = delete;
    ~DBInstance() = delete;

    static bool Create(AB::Entities::GameInstance& inst);
    static bool Load(AB::Entities::GameInstance& inst);
    static bool Save(const AB::Entities::GameInstance& inst);
    static bool Delete(const AB::Entities::GameInstance& inst);
    static bool Exists(const AB::Entities::GameInstance& inst);
};

}
