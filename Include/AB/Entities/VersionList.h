#pragma once

#include <AB/Entities/Entity.h>
//include inheritance extension
//this header contains two extensions, that specifies inheritance type of base class
//  BaseClass - normal inheritance
//  VirtualBaseClass - when virtual inheritance is used
//in order for virtual inheritance to work, InheritanceContext is required.
//it can be created either internally (via configuration) or externally (pointer to context).
#include <bitsery/ext/inheritance.h>
#include <AB/Entities/Limits.h>
#include "Version.h"

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

static constexpr auto KEY_VERSIONLIST = "version_list";

struct VersionList : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_VERSIONLIST;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.container(versions, Limits::MAX_VERSIONS, [&s](Version& c)
        {
            c.serialize(s);
        });
    }

    std::vector<Version> versions;
};

}
}
