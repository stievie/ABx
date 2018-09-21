#pragma once

#include <vector>
#include "StringHash.h"
#include <uuids.h>

class DataKey
{
public:
    DataKey() noexcept = default;

    DataKey(const DataKey& other) noexcept :
        data_(other.data_)
    { }
    DataKey(const std::string& table, const uuids::uuid& id) noexcept
    {
        data_.assign(table.begin(), table.end());
        data_.insert(data_.end(), id.begin(), id.end());
    }
    explicit DataKey(const std::string& key) noexcept :
        data_(key.begin(), key.end())
    { }
    ~DataKey() noexcept = default;

    DataKey& operator =(const DataKey& other)
    {
        data_ = other.data_;
        return *this;
    }

    inline size_t size() const noexcept
    {
        return data_.size();
    }
    inline const uint8_t* data() const noexcept
    {
        return data_.data();
    }
    inline uint8_t* data() noexcept
    {
        return data_.data();
    }

    inline void resize(size_t new_size)
    {
        data_.resize(new_size);
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
        if (!decode(table, id))
            return "";
        return table + id.to_string();
    }

    std::vector<uint8_t> data_;
};

inline bool operator==(const DataKey& lhs, const DataKey& rhs)
{
    return (lhs.data_ == rhs.data_);
}

namespace std
{
template<> struct hash<DataKey>
{
    typedef DataKey argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& s) const noexcept
    {
        return Utils::StringHashRt((const char*)s.data(), s.size());
    }
};
}
