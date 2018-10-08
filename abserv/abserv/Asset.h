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

}
