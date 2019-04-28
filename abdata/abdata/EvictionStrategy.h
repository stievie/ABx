#pragma once

#include <string>
#include <multi_index_container.hpp>
#include <multi_index/hashed_index.hpp>
#include <multi_index/member.hpp>
#include <multi_index/identity.hpp>
#include <multi_index/ordered_index.hpp>
#include "DataKey.h"

class DataItem
{
public:
    IO::DataKey key;
    uint64_t rank;
    DataItem(const IO::DataKey& k, uint64_t r) :
        key(k),
        rank(r)
    { }
    bool operator<(const DataItem& e) const
    {
        return rank < e.rank;
    }
};

/*namespace boost
{
template<> struct hash<IO::DataKey>
{
    typedef IO::DataKey argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& p) const noexcept
    {
        return Utils::StringHashRt((const char*)p.data(), p.size());
    }
};
} */

typedef multi_index::multi_index_container
<
    DataItem,
    multi_index::indexed_by
    <
        multi_index::hashed_unique<multi_index::member<DataItem, IO::DataKey, &DataItem::key>>,
        multi_index::ordered_unique<multi_index::member<DataItem, uint64_t, &DataItem::rank>>
    >
> DataItemContainer;

class  EvictionStrategy
{
public:
    virtual ~EvictionStrategy() = default;
    virtual IO::DataKey NextEviction() = 0;
    virtual void AddKey(const IO::DataKey&) = 0;
    virtual void RefreshKey(const IO::DataKey&) = 0;
    virtual void DeleteKey(const IO::DataKey&) = 0;
    virtual void Clear() = 0;
};

class OldestInsertionEviction : public EvictionStrategy
{
public:
    OldestInsertionEviction() :
        currentRank_(0)
    { }
    IO::DataKey NextEviction() override;
    void AddKey(const IO::DataKey&) override;
    void RefreshKey(const IO::DataKey&) override;
    void DeleteKey(const IO::DataKey&) override;
    void Clear() override;
private:
    uint64_t GetNextRank()
    {
        //NOTE not thread safe
        return currentRank_++;
    }
    uint64_t currentRank_;
    DataItemContainer dataItems_;
};