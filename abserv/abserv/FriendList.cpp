#include "stdafx.h"
#include "FriendList.h"
#include "Subsystems.h"
#include "DataClient.h"
#include <AB/Entities/Character.h>
#include "UuidUtils.h"
#include "StringUtils.h"
#include <sa/Transaction.h>

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
    if (Exists(accountUuid))
        return FriendList::Error::AlreadyFriend;

    sa::Transaction transaction(friendList_);
    friendList_.friends.push_back({
        accountUuid,
        name,
        relation,
        Utils::Tick()
    });
    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Update(friendList_))
        return FriendList::Error::InternalError;
    transaction.Commit();
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

FriendList::Error FriendList::Remove(const std::string& accountUuid)
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
    sa::Transaction transaction(friendList_);
    friendList_.friends.erase(it);
    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Update(friendList_))
        return FriendList::Error::InternalError;
    transaction.Commit();
    return FriendList::Error::Success;
}

FriendList::Error FriendList::ChangeNickname(const std::string& currentName, const std::string& newName)
{
    auto it = std::find_if(friendList_.friends.begin(), friendList_.friends.end(), [&](const AB::Entities::Friend& current)
    {
        return Utils::StringEquals(currentName, current.friendName);
    });
    if (it == friendList_.friends.end())
        return FriendList::Error::PlayerNotFound;

    sa::Transaction transaction(friendList_);
    (*it).friendName = newName;

    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Update(friendList_))
        return FriendList::Error::InternalError;
    transaction.Commit();
    return FriendList::Error::Success;
}

bool FriendList::Exists(const std::string& accountUuid)
{
    auto it = std::find_if(friendList_.friends.begin(), friendList_.friends.end(), [&](const AB::Entities::Friend& current)
    {
        return Utils::Uuid::IsEqual(accountUuid, current.friendUuid);
    });
    return (it != friendList_.friends.end());
}

bool FriendList::IsFriend(const std::string& accountUuid)
{
    bool result = false;
    VisitAll([&accountUuid, &result](const AB::Entities::Friend& current)
    {
        if (Utils::Uuid::IsEqual(accountUuid, current.friendUuid) && (current.relation == AB::Entities::FriendRelationFriend))
        {
            result = true;
            return Iteration::Break;
        }
        return Iteration::Continue;
    });
    return result;
}

bool FriendList::IsIgnored(const std::string& accountUuid)
{
    bool result = false;
    VisitAll([&accountUuid, &result](const AB::Entities::Friend& current)
    {
        if (Utils::Uuid::IsEqual(accountUuid, current.friendUuid) && (current.relation == AB::Entities::FriendRelationIgnore))
        {
            result = true;
            return Iteration::Break;
        }
        return Iteration::Continue;
    });
    return result;
}

bool FriendList::GetFriendByName(const std::string& name, AB::Entities::Friend& f)
{
    bool result = false;
    VisitAll([&](const AB::Entities::Friend& current)
    {
        if (Utils::SameName(name, current.friendName))
        {
            f.creation = current.creation;
            f.friendName = current.friendName;
            f.friendUuid = current.friendUuid;
            f.relation = current.relation;
            result = true;
            return Iteration::Break;
        }
        return Iteration::Continue;
    });

    return result;
}

bool FriendList::GetFriendByAccount(const std::string& accountUuid, AB::Entities::Friend& f)
{
    bool result = false;
    VisitAll([&](const AB::Entities::Friend& current)
    {
        if (Utils::Uuid::IsEqual(accountUuid, current.friendUuid))
        {
            f.creation = current.creation;
            f.friendName = current.friendName;
            f.friendUuid = current.friendUuid;
            f.relation = current.relation;
            result = true;
            return Iteration::Break;
        }
        return Iteration::Continue;
    });

    return result;
}

}
