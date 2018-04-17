#pragma once

#include <stdint.h>
//include inheritance extension
//this header contains two extensions, that specifies inheritance type of base class
//  BaseClass - normal inheritance
//  VirtualBaseClass - when virtual inheritance is used
//in order for virtual inheritance to work, InheritanceContext is required.
//it can be created either internally (via configuration) or externally (pointer to context).
#include <bitsery/ext/inheritance.h>

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

struct Entity
{
    template<typename S>
    void serialize(S& s)
    {
        s.value4b(id);
    }

    uint32_t id = 0;
};

}
}
