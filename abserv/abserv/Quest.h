#pragma once

#include <stdint.h>

namespace Game {

class Quest
{
public:
    Quest() = default;
    // non-copyable
    Quest(const Quest&) = delete;
    Quest& operator=(const Quest&) = delete;

    ~Quest() = default;

    void Update(uint32_t timeElapsed);
};

}
