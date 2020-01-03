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
        InternalError
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
