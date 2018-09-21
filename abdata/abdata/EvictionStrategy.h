#pragma once

#include <string>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "DataKey.h"

class DataItem
{
public:
    DataKey key;
    uint64_t rank;
    DataItem(const DataKey& k, uint64_t r) :
        key(k),
        rank(r)
    { }
    bool operator<(const DataItem& e) const
    {
        return rank < e.rank;
    }
};

namespace boost
{
template<> struct hash<DataKey>
{
    typedef DataKey argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& p) const noexcept
    {
        return Utils::StringHashRt((const char*)p.data(), p.size());
    }
};
}

typedef boost::multi_index::multi_index_container
<
    DataItem,
    boost::multi_index::indexed_by
    <
        boost::multi_index::hashed_unique<boost::multi_index::member<DataItem, DataKey, &DataItem::key>>,
        boost::multi_index::ordered_unique<boost::multi_index::member<DataItem, uint64_t, &DataItem::rank>>
    >
> DataItemContainer;

class  EvictionStrategy
{
public:
    virtual ~EvictionStrategy() = default;
    virtual DataKey NextEviction() = 0;
    virtual void AddKey(const DataKey&) = 0;
    virtual void RefreshKey(const DataKey&) = 0;
    virtual void DeleteKey(const DataKey&) = 0;
};

class OldestInsertionEviction : public EvictionStrategy
{
public:
    OldestInsertionEviction();
    DataKey NextEviction() override;
    void AddKey(const DataKey&) override;
    void RefreshKey(const DataKey&) override;
    void DeleteKey(const DataKey&) override;
private:
    uint64_t GetNextRank()
    {
        //NOTE not thread safe
        return currentRank_++;
    }
    uint64_t currentRank_;
    DataItemContainer dataItems_;
};