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

inline constexpr uint16_t CHEST_SLOT_INCREASE = 10;

enum AccountKeyStatus : uint8_t
{
    KeyStatusUnknown = 0,
    KeyStatusNotActivated = 1,
    KeryStatusReadyForUse = 2,
    KeyStatusBanned = 3
};
enum AccountKeyType : uint8_t
{
    KeyTypeUnknown = 0,
    KeyTypeAccount = 1,
    KeyTypeCharSlot = 2,
    KeyTypeChestSlots = 3,     // Increase count of player inventory by 10
};

/// Account key entity. The UUID is the key.
struct AccountKey : Entity
{
    MAKE_ENTITY(AccountKey)
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value1b(type);
        s.value2b(total);
        s.value2b(used);
        s.value1b(status);
        s.text1b(email, Limits::MAX_ACCOUNT_EMAIL);
        s.text1b(description, Limits::MAX_ACCOUNTKEY_DESCRIPTION);
    }

    AccountKeyType type = KeyTypeUnknown;
    uint16_t total = 1;
    uint16_t used = 0;
    AccountKeyStatus status = KeyStatusUnknown;
    std::string email;
    std::string description;
};

}
}
