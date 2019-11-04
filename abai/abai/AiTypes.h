#pragma once

#include <stdint.h>
#include <limits>
#include <vector>
#include <string>

namespace AI {

typedef uint32_t Id;
constexpr Id INVALID_ID = std::numeric_limits<Id>::max();
using ArgumentsType = std::vector<std::string>;

}