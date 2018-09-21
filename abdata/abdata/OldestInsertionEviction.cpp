#include "stdafx.h"
#include "EvictionStrategy.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>

using namespace boost::multi_index;

OldestInsertionEviction::OldestInsertionEviction() :
    currentRank_(0)
{
}

DataKey OldestInsertionEviction::NextEviction()
{
    DataItemContainer::nth_index<1>::type& rankIndex = dataItems_.get<1>();

    auto first = rankIndex.begin();
    const DataKey& retVal = first->key;
    dataItems_.erase(retVal);
    return retVal;
}

void OldestInsertionEviction::AddKey(const DataKey& key)
{
    dataItems_.insert(DataItem(key, GetNextRank()));
}

void OldestInsertionEviction::RefreshKey(const DataKey& key)
{
    uint64_t next = GetNextRank();
    auto keyItr = dataItems_.find(key);
    dataItems_.modify(keyItr, [next](DataItem& d)
    {
        d.rank = next;
    });
}

void OldestInsertionEviction::DeleteKey(const DataKey& key)
{
    auto keyItr = dataItems_.find(key);
    if (keyItr != dataItems_.end())
    {
        dataItems_.erase(keyItr);
    }

}
