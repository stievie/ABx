#pragma once

namespace IO {

class Asset
{
public:
    Asset() :
        fileName_("")
    {}
    virtual ~Asset() = default;

    std::string fileName_;
};

template<class T>
class AssetImpl : public Asset, public std::enable_shared_from_this<T>
{
};

}
