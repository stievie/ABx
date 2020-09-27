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

#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>

namespace IO {

class IOAccount
{
public:
    enum class PasswordAuthResult
    {
        OK = 0,
        InvalidAccount,
        AlreadyLoggedIn,
        PasswordMismatch,
        InternalError,
        AccountBanned
    };
    enum AccountKeyStatus : uint8_t
    {
        NotActivated = 0,
        ReadyForUse = 1,
        Banned = 2
    };
    enum AccountKeyType : uint8_t
    {
        KeyTypeAccount = 0,
        KeyTypeCharSlot = 1,
    };
    enum class CreateAccountResult
    {
        OK = 0,
        NameExists,
        InvalidAccountKey,
        InvalidAccount,
        InternalError,
        EmailError,
        PasswordError,
        AlreadyAdded,
    };
    enum class CreatePlayerResult
    {
        OK = 0,
        NameExists,
        InvalidAccount,
        NoMoreCharSlots,
        InvalidProfession,
        InternalError,
        InvalidName
    };
    IOAccount() = delete;
    static CreateAccountResult CreateAccount(const std::string& name, const std::string& pass,
        const std::string& email, const std::string& accKey);
    static CreateAccountResult AddAccountKey(AB::Entities::Account& account,
        const std::string& accKey);
    static IOAccount::PasswordAuthResult PasswordAuth(const std::string& pass,
        AB::Entities::Account& account);
    static bool TokenAuth(const std::string& token,
        AB::Entities::Account& account);
    static IOAccount::CreatePlayerResult CreatePlayer(const std::string& accountUuid,
        const std::string& name, const std::string& profUuid,
        uint32_t modelIndex,
        AB::Entities::CharacterSex sex, bool isPvp,
        std::string& uuid);
    static bool LoadCharacter(AB::Entities::Character& ch);
    static bool DeletePlayer(const std::string& accountUuid, const std::string& playerUuid);
    static bool IsNameAvailable(const std::string& name, const std::string& forAccountUuid);
};

}
