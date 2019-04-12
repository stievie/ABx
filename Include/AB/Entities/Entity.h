#pragma once

#include <stdint.h>
#include <string>
//include inheritance extension
//this header contains two extensions, that specifies inheritance type of base class
//  BaseClass - normal inheritance
//  VirtualBaseClass - when virtual inheritance is used
//in order for virtual inheritance to work, InheritanceContext is required.
//it can be created either internally (via configuration) or externally (pointer to context).
#include <bitsery/ext/inheritance.h>
#include <AB/Entities/Limits.h>

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

static constexpr uint32_t INVALID_INDEX = std::numeric_limits<uint32_t>::max();
static constexpr auto EMPTY_GUID = "00000000-0000-0000-0000-000000000000";

/// Base class for entities.
struct Entity
{
    template<typename S>
    void serialize(S& s)
    {
        // UUID must be the first serialized
        s.text1b(uuid, Limits::MAX_UUID);
    }

    std::string uuid = EMPTY_GUID;
};

}
}
