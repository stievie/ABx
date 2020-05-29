/**
 * Copyright 2020 Stefan Ascher
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
