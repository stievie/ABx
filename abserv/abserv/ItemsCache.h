#pragma once

#include <unordered_map>
#include <sa/IdGenerator.h>
#include "Item.h"

namespace Game {

class ItemsCache
{
private:
    static sa::IdGenerator<uint32_t> itemIds_;
    std::unordered_map<uint32_t, std::unique_ptr<Item>> itemCache_;
    std::unordered_map<std::string, uint32_t> concreteIds_;
public:
    Item* Get(uint32_t id);
    uint32_t GetConcreteId(const std::string& uuid) const;
    uint32_t Add(std::unique_ptr<Item>&& item);
    void Remove(uint32_t id);
    void RemoveConcrete(const std::string& uuid);
};

}
