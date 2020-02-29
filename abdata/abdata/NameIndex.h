#pragma once

#include <multi_index_container.hpp>
#include <multi_index/hashed_index.hpp>
#include <multi_index/member.hpp>
#include <multi_index/identity.hpp>
#include <multi_index/ordered_index.hpp>
#include <abscommon/DataKey.h>
#include <string>

class NameIndex
{
private:
    struct IndexItem
    {
        IO::DataKey key;
        size_t tableHash;
        std::string name;
    };
    using IndexContainer = multi_index::multi_index_container
    <
        IndexItem,
        multi_index::indexed_by
        <
            multi_index::hashed_unique<multi_index::member<IndexItem, IO::DataKey, &IndexItem::key>>,
            multi_index::ordered_non_unique<multi_index::member<IndexItem, size_t, &IndexItem::tableHash>>,
            multi_index::hashed_non_unique<multi_index::member<IndexItem, std::string, &IndexItem::name>>
        >
    >;
    IndexContainer index_;
public:
    void Add(const IO::DataKey& key, const std::string& name);
    void Delete(const IO::DataKey& key);
    const IO::DataKey* LookupName(size_t table, const std::string& name) const;
    void Clear();
};
