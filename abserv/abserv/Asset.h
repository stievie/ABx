#pragma once

#include <string>

namespace IO {

class Asset
{
public:
    Asset() :
        fileName_("")
    {}
    virtual ~Asset();

    std::string fileName_;
};

}
