/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <multi_index_container.hpp>
#include <multi_index/hashed_index.hpp>
#include <multi_index/member.hpp>
#include <multi_index/identity.hpp>
#include <multi_index/ordered_index.hpp>
#include <abscommon/DataKey.h>

class CacheIndex
{
private:
    struct DataItem
    {
        IO::DataKey key;
        uint64_t rank;
        bool operator<(const DataItem& e) const
        {
            return rank < e.rank;
        }
    };
    using DataItemContainer = multi_index::multi_index_container
    <
        DataItem,
        multi_index::indexed_by
        <
            multi_index::hashed_unique<multi_index::member<DataItem, IO::DataKey, &DataItem::key>>,
            multi_index::ordered_unique<multi_index::member<DataItem, uint64_t, &DataItem::rank>>
        >
    >;
public:
    CacheIndex() :
        currentRank_(0)
    { }
    IO::DataKey Next();
    void Add(const IO::DataKey&);
    void Refresh(const IO::DataKey&);
    void Delete(const IO::DataKey&);
    void Clear();
private:
    uint64_t GetNextRank()
    {
        // NOTE: Not thread safe
        return currentRank_++;
    }
    uint64_t currentRank_;
    DataItemContainer dataItems_;
};
