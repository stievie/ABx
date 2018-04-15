#pragma once

#include <string>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/ordered_index.hpp>

class DataItem
{
public:
    std::string key;
    uint64_t rank;
    DataItem(const std::string& k, uint64_t r) :
        key(k),
        rank(r)
    { }
    bool operator<(const DataItem& e) const
    {
        return rank < e.rank;
    }
};

typedef boost::multi_index::multi_index_container
<
    DataItem,
    boost::multi_index::indexed_by
    <
        boost::multi_index::hashed_unique<boost::multi_index::member<DataItem, std::string, &DataItem::key>>,
        boost::multi_index::ordered_unique<boost::multi_index::member<DataItem, uint64_t, &DataItem::rank>>
    >
> DataItemContainer;

class  EvictionStrategy
{
public:
    virtual ~EvictionStrategy() = default;
    virtual std::string NextEviction() = 0;
    virtual void AddKey(const std::string&) = 0;
    virtual void RefreshKey(const std::string&) = 0;
    virtual void DeleteKey(const std::string&) = 0;
};

class OldestInsertionEviction : public EvictionStrategy
{
public:
    OldestInsertionEviction();
    std::string NextEviction();
    void AddKey(const std::string&) override;
    void RefreshKey(const std::string&) override;
    void DeleteKey(const std::string&) override;
private:
    uint64_t GetNextRank()
    {
        //NOTE not thread safe
        return currentRank_++;
    }
    uint64_t currentRank_;
    DataItemContainer dataItems_;
};