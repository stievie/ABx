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

namespace AB {
namespace Entities {

enum BanReason : uint8_t
{
    BanReasonUnknown = 0,
    BanReasonOther,
    BanReasonScamming,
    BanReasonBotting,
    BanReasonGoldSelling,
    BanReasonNameReport,
};

struct Ban : Entity
{
    MAKE_ENTITY(Ban)
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value8b(expires);
        s.value8b(added);
        s.value1b(reason);
        s.value1b(active);
        s.text1b(adminUuid, Limits::MAX_UUID);
        s.text1b(comment, Limits::MAX_BAN_COMMENT);
    }

    timestamp_t expires = 0;
    timestamp_t added = 0;
    BanReason reason = BanReasonUnknown;
    bool active = false;
    std::string adminUuid = EMPTY_GUID;
    std::string comment;
    uint32_t hits = 0;
};

}
}
