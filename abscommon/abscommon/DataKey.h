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

#include <vector>
#include <sa/StringHash.h>
#include <uuid.h>
#include <string_view>

namespace IO {

/// Key used for data server cache.
class DataKey
{
public:
    DataKey() noexcept = default;

    DataKey(const DataKey& other) noexcept :
        data_(other.data_)
    { }
    DataKey(DataKey&& other) noexcept :
        data_(std::move(other.data_))
    { }
    DataKey(std::string_view table, const uuids::uuid& id) noexcept
    {
        data_.assign(table.begin(), table.end());
        data_.insert(data_.end(), id.begin(), id.end());
    }
    explicit DataKey(std::string_view key) noexcept :
        data_(key.begin(), key.end())
    { }
    ~DataKey() noexcept = default;

    /// Assignment
    DataKey& operator =(const DataKey& other) noexcept
    {
        if (this != &other)
            data_ = other.data_;
        return *this;
    }
    DataKey& operator =(DataKey&& other) noexcept
    {
        if (this != &other)
            data_ = std::move(other.data_);
        return *this;
    }

    size_t size() const noexcept
    {
        return data_.size();
    }
    const uint8_t* data() const noexcept
    {
        return data_.data();
    }
    uint8_t* data() noexcept
    {
        return data_.data();
    }

    void resize(size_t new_size)
    {
        data_.resize(new_size);
    }
    std::string_view table() const
    {
        if (data_.size() <= uuids::uuid::state_size)
            return {};
        return std::string_view(reinterpret_cast<const char*>(data_.data()),
            static_cast<size_t>(data_.end() - data_.begin()) - uuids::uuid::state_size);
    }
    bool decode(std::string& table, uuids::uuid& id) const
    {
        // key = <tablename><guid>
        if (data_.size() <= uuids::uuid::state_size)
            return false;
        table.assign(data_.begin(), data_.end() - uuids::uuid::state_size);
        id = uuids::uuid(data_.end() - uuids::uuid::state_size, data_.end());
        return true;
    }
    /// Pretty print key
    std::string format() const
    {
        std::string table;
        uuids::uuid id;
        if (decode(table, id))
            return table + id.to_string();
        return std::string(data_.begin(), data_.end());
    }

    std::vector<uint8_t> data_;
};

inline bool operator==(const DataKey& lhs, const DataKey& rhs)
{
    return (lhs.data_ == rhs.data_);
}

}

namespace std
{
template<> struct hash<IO::DataKey>
{
    typedef IO::DataKey argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& s) const noexcept
    {
        return sa::StringHashRt(reinterpret_cast<const char*>(s.data()), s.size());
    }
};
}
