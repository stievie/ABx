/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_RESERVED_NAMES = "reserved_names";

struct ReservedName : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_RESERVED_NAMES;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(name, Limits::MAX_RESERVED_NAME);
        s.value1b(isReserved);
        s.text1b(reservedForAccountUuid, Limits::MAX_UUID);
        s.value8b(expires);
    }

    std::string name;
    bool isReserved = false;
    std::string reservedForAccountUuid = EMPTY_GUID;
    timestamp_t expires = 0;
};

}
}
