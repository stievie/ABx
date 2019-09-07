#include "stdafx.h"
#include "FriendList.h"
#include "Subsystems.h"
#include "DataClient.h"
#include <AB/Entities/Character.h>
#include "UuidUtils.h"
#include "StringUtils.h"

namespace Game {

void FriendList::Load()
{
    auto* client = GetSubsystem<IO::DataClient>();
    friendList_.uuid = accountUuid_;
    friendList_.friends.clear();
    if (!client->Read(friendList_))
    {
        LOG_WARNING << "Error reading Friendlist for account " << accountUuid_ << std::endl;
    }
}

FriendList::Error FriendList::AddFriendAccount(const std::string& accountUuid,
    const std::string& name,
    AB::Entities::FriendRelation relation)
{
    const auto it = std::find_if(friendList_.friends.begin(), friendList_.friends.end(), [&](const AB::Entities::Friend& current)
    {
        return Utils::Uuid::IsEqual(accountUuid, current.friendUuid);
    });
    if (it != friendList_.friends.end())
        return FriendList::Error::AlreadyFriend;

    friendList_.friends.push_back({
        accountUuid,
        name,
        relation
    });
    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Update(friendList_))
        return FriendList::Error::InternalError;
    return FriendList::Error::Success;
}

FriendList::Error FriendList::AddFriendByName(const std::string& playerName, AB::Entities::FriendRelation relation)
{
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Character ch;
    ch.name = playerName;
    if (!client->Read(ch))
    {
        return FriendList::Error::PlayerNotFound;
    }
    return AddFriendAccount(ch.accountUuid, playerName, relation);
}

FriendList::Error FriendList::RemoveByAccount(const std::string& accountUuid)
{
    if (Utils::Uuid::IsEqual(accountUuid_, accountUuid))
        // Can not add self as friend
        return FriendList::Error::PlayerNotFound;

    auto it = std::find_if(friendList_.friends.begin(), friendList_.friends.end(), [&](const AB::Entities::Friend& current)
    {
        return Utils::Uuid::IsEqual(accountUuid, current.friendUuid);
    });
    if (it == friendList_.friends.end())
        return FriendList::Error::NoFriend;
    friendList_.friends.erase(it);
    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Update(friendList_))
        return FriendList::Error::InternalError;
    return FriendList::Error::Success;
}

FriendList::Error FriendList::Remove(const std::string& playerName)
{
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Character ch;
    ch.name = playerName;
    if (!client->Read(ch))
    {
        return FriendList::Error::PlayerNotFound;
    }
    return RemoveByAccount(ch.accountUuid);
}

FriendList::Error FriendList::ChangeNickname(const std::string& currentName, const std::string& newName)
{
    auto it = std::find_if(friendList_.friends.begin(), friendList_.friends.end(), [&](const AB::Entities::Friend& current)
    {
        return Utils::StringEquals(currentName, current.friendName);
    });
    if (it == friendList_.friends.end())
        return FriendList::Error::PlayerNotFound;

    (*it).friendName = newName;

    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Update(friendList_))
        return FriendList::Error::InternalError;
    return FriendList::Error::Success;
}

}
