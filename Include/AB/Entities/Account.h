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
#include <bitsery/traits/vector.h>
#include <bitsery/traits/array.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_ACCOUNTS = "accounts";
static const uint32_t ACCOUNT_DEF_CHARSLOTS = 6;
// Account chest size
static constexpr size_t DEFAULT_CHEST_SIZE = 80;

enum AccountType : uint8_t
{
    AccountTypeUnknown = 0,
    AccountTypeNormal = 1,
    AccountTypeTutor = 2,
    AccountTypeSeniorTutor = 3,
    AccountTypeGamemaster = 4,
    AccountTypeGod = 5
};

enum AccountStatus : uint8_t
{
    AccountStatusUnknown = 0,
    AccountStatusActivated = 1,
    AccountStatusDeleted = 2
};

enum OnlineStatus : uint8_t
{
    OnlineStatusOffline = 0,
    OnlineStatusAway,
    OnlineStatusDoNotDisturb,
    OnlineStatusOnline,
    OnlineStatusInvisible              // Like offline for other users
};

inline bool IsOnline(OnlineStatus status)
{
    // OnlineStatusOfflineis used internally. The user can use OnlineStatusInvisible
    // to appear to others as offline.
    return status != OnlineStatusOffline && status != OnlineStatusInvisible;
}

struct Account : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_ACCOUNTS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value1b(type);
        s.value1b(status);
        s.value8b(creation);
        s.text1b(name, Limits::MAX_ACCOUNT_NAME);
        s.text1b(password, Limits::MAX_ACCOUNT_PASS);
        s.text1b(email, Limits::MAX_ACCOUNT_EMAIL);
        s.text1b(authToken, Limits::MAX_UUID);
        s.value8b(authTokenExpiry);
        s.text1b(currentServerUuid, Limits::MAX_UUID);
        s.value4b(charSlots);
        s.text1b(currentCharacterUuid, Limits::MAX_UUID);
        s.value1b(onlineStatus);
        s.text1b(guildUuid, Limits::MAX_UUID);
        s.container1b(clientPubKey);

        // https://github.com/fraillt/bitsery/blob/master/examples/context_usage.cpp
        // https://github.com/fraillt/bitsery/blob/master/examples/basic_usage.cpp
        s.container(characterUuids, Limits::MAX_ACCOUNT_CHARACTERS, [&s](std::string& c)
        {
            s.text1b(c, Limits::MAX_UUID);
        });
    }

    AccountType type = AccountTypeUnknown;
    AccountStatus status = AccountStatusUnknown;
    timestamp_t creation = 0;
    std::string name;
    std::string password;
    std::string email;
    std::string authToken;
    timestamp_t authTokenExpiry;
    /// The server currently logged in. Required for cross server chat etc.
    std::string currentServerUuid = EMPTY_GUID;
    uint32_t charSlots = ACCOUNT_DEF_CHARSLOTS;
    /// Last or current character
    std::string currentCharacterUuid = EMPTY_GUID;
    std::vector<std::string> characterUuids;
    OnlineStatus onlineStatus = OnlineStatusOffline;
    std::string guildUuid = EMPTY_GUID;
    // Clients DH public key (16 byte)
    std::array<uint8_t, 16> clientPubKey;
    uint16_t chest_size = DEFAULT_CHEST_SIZE;
};

}
}
