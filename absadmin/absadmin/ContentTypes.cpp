#include "stdafx.h"
#include "ContentTypes.h"

const std::string& ContentTypes::Get(const std::string& ext) const
{
    static std::string def = "application/octet-stream";
    auto it = map_.find(ext);
    if (it == map_.end())
        return def;
    return (*it).second;
}
