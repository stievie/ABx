#pragma once

#include <AB/Entities/ConcreteItem.h>

class StorageProvider;

namespace DB {

class DBConcreteItem
{
public:
    DBConcreteItem() = delete;
    ~DBConcreteItem() = delete;

    static bool Create(AB::Entities::ConcreteItem& item);
    static bool Load(AB::Entities::ConcreteItem& item);
    static bool Save(const AB::Entities::ConcreteItem& item);
    static bool Delete(const AB::Entities::ConcreteItem& item);
    static bool Exists(const AB::Entities::ConcreteItem& item);
    /// Delete not picked up items
    static void Clean(StorageProvider* sp);
};

}
