#pragma once

#include <stdint.h>

namespace Utils {

uint32_t AdlerChecksum(uint8_t* data, int32_t len);
}
