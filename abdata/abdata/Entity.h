#pragma once

#include <stdint.h>

namespace Entities {

struct Entity
{
    uint32_t id;
    bool dirty;
};

}