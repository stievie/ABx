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

#include "stdafx.h"
#include "IOAccount.h"
#include "IOGame.h"
#include <AB/Entities/AccountKey.h>
#include <AB/Entities/AccountKeyAccounts.h>
#include <AB/Entities/Character.h>
#include <abscommon/UuidUtils.h>

namespace IO {
namespace IOAccount {

bool GameWorldAuth(const std::string& accountUuid, const std::string& authToken,
    const std::string& charUuid)
{
    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Account acc;
    acc.uuid = accountUuid;
    if (!client->Read(acc))
    {
        // At this point the account must exist.
        LOG_ERROR << "Error reading account " << accountUuid << std::endl;
        return false;
    }
    if (!Utils::Uuid::IsEqual(authToken, acc.authToken))
    {
        LOG_WARNING << "Wrong auth token " << authToken << " expected " << acc.authToken << std::endl;
        return false;
    }
    if (Utils::IsExpired(acc.authTokenExpiry))
    {
        LOG_INFO << "Expired auth token " << authToken << " of account " << acc.name << std::endl;
        return false;
    }

    AB::Entities::Character ch;
    ch.uuid = charUuid;
    if (!client->Read(ch))
    {
        LOG_ERROR << "Error reading character " << charUuid << std::endl;
        return false;
    }
    if (!Utils::Uuid::IsEqual(ch.accountUuid, acc.uuid))
    {
        LOG_ERROR << "Character " << charUuid << " does not belong to account " << accountUuid << std::endl;
        return false;
    }

    return true;
}

bool AccountLogout(const std::string& uuid)
{
    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Account acc;
    acc.uuid = uuid;
    if (!client->Read(acc))
    {
        LOG_ERROR << "Error reading account " << uuid << std::endl;
        return false;
    }
    acc.onlineStatus = AB::Entities::OnlineStatus::OnlineStatusOffline;
    return client->Update(acc);
}

bool GetAccountInfo(AB::Entities::Account& account, AB::Entities::Character& character)
{
    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Read(account))
        return false;
    if (Utils::Uuid::IsEmpty(account.currentCharacterUuid))
        return false;
    character.uuid = account.currentCharacterUuid;
    if (!client->Read(character))
        return false;
    return true;
}

bool GetGuildMemberInfo(const AB::Entities::Account& account, AB::Entities::GuildMember& g)
{
    if (Utils::Uuid::IsEmpty(account.guildUuid))
        return false;
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::GuildMembers gms;
    gms.uuid = account.guildUuid;
    if (!client->Read(gms))
        return false;
    const auto it = std::find_if(gms.members.begin(), gms.members.end(), [&](const AB::Entities::GuildMember& current)
    {
        return Utils::Uuid::IsEqual(account.uuid, current.accountUuid);
    });
    if (it == gms.members.end())
        return false;
    g = (*it);
    return true;
}

}
}
