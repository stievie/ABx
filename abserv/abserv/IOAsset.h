#pragma once

namespace IO {

class IOAsset
{
public:
    IOAsset() = default;
    virtual ~IOAsset() = default;
};

template<class T>
class IOAssetImpl : public IOAsset
{
public:
    virtual bool Import(T& asset, const std::string& name) = 0;
};

}
