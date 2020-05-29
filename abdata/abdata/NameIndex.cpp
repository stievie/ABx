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
