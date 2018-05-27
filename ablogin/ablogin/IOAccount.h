#pragma once

#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>

namespace IO {

class IOAccount
{
public:
    enum LoginError
    {
        LoginOK = 0,
        LoginInvalidAccount,
        LoginPasswordMismatch
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
    enum Result
    {
        ResultOK,
        ResultNameExists,
        ResultInvalidAccountKey,
        ResultInvalidAccount,
        ResultInternalError
    };
    enum CreatePlayerResult
    {
        CreatePlayerResultOK,
        CreatePlayerResultNameExists,
        CreatePlayerResultInvalidAccount,
        CreatePlayerResultNoMoreCharSlots,
        CreatePlayerResultInvalidProfession,
        CreatePlayerResultInternalError
    };
    IOAccount() = delete;
    static Result CreateAccount(const std::string& name, const std::string& pass,
        const std::string& email, const std::string& accKey);
    static Result AddAccountKey(const std::string& accountUuid, const std::string& pass,
        const std::string& accKey);
    static IOAccount::LoginError LoginServerAuth(const std::string& pass,
        AB::Entities::Account& account);
    static IOAccount::CreatePlayerResult CreatePlayer(const std::string& accountUuid,
        const std::string& name, const std::string& profUuid, AB::Entities::CharacterSex sex, bool isPvp,
        std::string& uuid);
    static bool LoadCharacter(AB::Entities::Character& ch);
    static bool DeletePlayer(const std::string& accountUuid, const std::string& playerUuid);
};

}
