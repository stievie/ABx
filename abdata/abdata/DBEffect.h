#pragma once

#include <AB/Entities/Effect.h>

namespace DB {

class DBEffect
{
public:
    DBEffect() = delete;
    ~DBEffect() = delete;

    static bool Create(AB::Entities::Effect& effect);
    static bool Load(AB::Entities::Effect& effect);
    static bool Save(const AB::Entities::Effect& effect);
    static bool Delete(const AB::Entities::Effect& effect);
    static bool Exists(const AB::Entities::Effect& effect);
};

}
