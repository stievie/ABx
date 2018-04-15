#pragma once

#include <stdint.h>
#include "Database.h"

namespace Entities {

struct Entity
{
    Entity() :
        id(0),
        dirty(false)
    {
    }
    uint32_t id;
    bool dirty;
};

template <typename S>
void serialize(S& s, Entity& o)
{
    s.value4b(o.id);
}

}