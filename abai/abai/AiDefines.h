#pragma once

#include <stdint.h>
#include <limits>
#include <vector>
#include <string>

#define ABAI_STRINGIFY_INTERNAL(x) #x
#define ABAI_STRINGIFY(x) ABAI_STRINGIFY_INTERNAL(x)

namespace AI {

typedef uint32_t Id;
constexpr Id INVALID_ID = std::numeric_limits<Id>::max();
using ArgumentsType = std::vector<std::string>;

}
