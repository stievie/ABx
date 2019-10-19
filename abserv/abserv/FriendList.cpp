#include "stdafx.h"
#include "FriendList.h"
#include "Subsystems.h"
#include "DataClient.h"
#include "UuidUtils.h"
#include "StringUtils.h"
#include <sa/Transaction.h>
#include "IOPlayer.h"
#include "Dispatcher.h"
#include <AB/Entities/FriendedMe.h>

namespace Game {

void FriendList::InvalidateList(const std::string& uuid)
{
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::FriendedMe f;
    f.uuid = uuid;
    client->Invalidate(f);
}

void FriendList::Load()
{
    auto* client = GetSubsystem<IO::DataClient>();
    data_.uuid = accountUuid_;
    data_.friends.clear();
    client->Read(data_);
    // If we can't read it this guy has no friends :/
}

void FriendList::Save()
{
    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Update(data_))
        LOG_ERROR << "Error saving friend list for account " << data_.uuid << std::endl;
}

FriendList::Error FriendList::AddFriendAccount(const AB::Entities::Character& ch,
    AB::Entities::FriendRelation relation)
{
    if (Utils::Uuid::IsEqual(accountUuid_, ch.accountUuid))
        // Can not add self as friend
        return FriendList::Error::PlayerNotFound;
    if (Exists(ch.accountUuid))
        return FriendList::Error::AlreadyFriend;

    data_.friends.push_back({
        ch.accountUuid,
        ch.name,
        relation,
        Utils::Tick()
    });

    InvalidateList(accountUuid_);
    InvalidateList(ch.accountUuid);

    GetSubsystem<Asynch::Dispatcher>()->Add(Asynch::CreateTask(std::bind(&FriendList::Save, this)));
    return FriendList::Error::Success;
}

FriendList::Error FriendList::AddFriendByName(const std::string& playerName, AB::Entities::FriendRelation relation)
{
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Character ch;
    ch.name = playerName;
    if (!client->Read(ch))
        return FriendList::Error::PlayerNotFound;
    return AddFriendAccount(ch, relation);
}

FriendList::Error FriendList::Remove(const std::string& accountUuid)
{
    if (Utils::Uuid::IsEqual(accountUuid_, accountUuid))
        // Can not add self as friend
        return FriendList::Error::PlayerNotFound;

    auto it = std::find_if(data_.friends.begin(), data_.friends.end(), [&](const AB::Entities::Friend& current)
    {
        return Utils::Uuid::IsEqual(accountUuid, current.friendUuid);
    });
    if (it == data_.friends.end())
        return FriendList::Error::NoFriend;
    data_.friends.erase(it);

    InvalidateList(accountUuid_);
    InvalidateList(accountUuid);

    GetSubsystem<Asynch::Dispatcher>()->Add(Asynch::CreateTask(std::bind(&FriendList::Save, this)));
    return FriendList::Error::Success;
}

FriendList::Error FriendList::ChangeNickname(const std::string& accountUuid, const std::string& newName)
{
    auto it = std::find_if(data_.friends.begin(), data_.friends.end(), [&](const AB::Entities::Friend& current)
    {
        return Utils::Uuid::IsEqual(accountUuid, current.friendUuid);
    });
    if (it == data_.friends.end())
        return FriendList::Error::PlayerNotFound;

    (*it).friendName = newName;

    GetSubsystem<Asynch::Dispatcher>()->Add(Asynch::CreateTask(std::bind(&FriendList::Save, this)));
    return FriendList::Error::Success;
}

bool FriendList::Exists(const std::string& accountUuid)
{
    auto it = std::find_if(data_.friends.begin(), data_.friends.end(), [&](const AB::Entities::Friend& current)
    {
        return Utils::Uuid::IsEqual(accountUuid, current.friendUuid);
    });
    return (it != data_.friends.end());
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

bool FriendList::IsIgnoredByName(const std::string& name)
{
    AB::Entities::Character ch;
    ch.name = name;
    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Read(ch))
        return false;
    return IsIgnored(ch.accountUuid);
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
