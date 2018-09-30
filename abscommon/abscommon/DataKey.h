#pragma once

#include <vector>
#include "StringHash.h"
#include <uuid.h>

namespace IO {

/// Key used for data server cache.
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

    /// Assignment
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
        return Utils::StringHashRt((const char*)s.data(), s.size());
    }
};
}
