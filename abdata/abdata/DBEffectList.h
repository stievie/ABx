#pragma once

#include <AB/Entities/EffectList.h>

namespace DB {

class DBEffectList
{
public:
    DBEffectList() = delete;
    ~DBEffectList() = delete;

    static bool Create(AB::Entities::EffectList&);
    static bool Load(AB::Entities::EffectList& el);
    static bool Save(const AB::Entities::EffectList&);
    static bool Delete(const AB::Entities::EffectList&);
    static bool Exists(const AB::Entities::EffectList&);
};

}
