#pragma once

#include <memory>

namespace IO {

class Asset
{
public:
    Asset();
    ~Asset();
};

template <typename T>
class AssetImpl : public Asset, public std::enable_shared_from_this<T>
{
public:
    std::shared_ptr<T> GetThis()
    {
        return shared_from_this();
    }
};

}
