
#include "NameIndex.h"
#include <sa/StringHash.h>
#include <abscommon/StringUtils.h>
#include <abscommon/Logger.h>

void NameIndex::Add(const IO::DataKey& key, const std::string& name)
{
    // Names are always case insensitive
    IndexItem item{ key, sa::StringHash(key.table()), Utils::Utf8ToLower(name) };
    index_.insert(std::move(item));
}

void NameIndex::Delete(const IO::DataKey& key)
{
    auto keyItr = index_.find(key);
    if (keyItr != index_.end())
        index_.erase(keyItr);
}

const IO::DataKey* NameIndex::LookupName(size_t table, const std::string& name) const
{
    auto& nameIndex = index_.get<2>();
    // Lookup by name first, there should be very little duplicate names across different tables.
    auto range = nameIndex.equal_range(Utils::Utf8ToLower(name));
    while (range.first != range.second)
    {
        if (range.first->tableHash == table)
            return &range.first->key;
        ++range.first;
    }

    return nullptr;
}

void NameIndex::Clear()
{
    index_.clear();
}
