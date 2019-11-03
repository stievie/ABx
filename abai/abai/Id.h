#pragma once

#include <stdint.h>
#include <limits>

namespace AI {

typedef uint32_t Id;
constexpr Id INVALID_ID = std::numeric_limits<Id>::max();

}