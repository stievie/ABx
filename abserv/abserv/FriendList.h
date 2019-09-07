#pragma once

#include <AB/Entities/FriendList.h>

namespace Game {

class FriendList
{
public:
    enum class Error
    {
        Success,
        PlayerNotFound,
        AlreadyFriend,
        NoFriend,
        InternalError
    };
private:
    std::string accountUuid_;
    AB::Entities::FriendList friendList_;
    FriendList::Error AddFriendAccount(const std::string& accountUuid, const std::string& name,
        AB::Entities::FriendRelation relation);
    FriendList::Error RemoveByAccount(const std::string& accountUuid);
public:
    FriendList(const std::string accountUuid) :
        accountUuid_(accountUuid),
        friendList_{}
    { }
    ~FriendList() = default;

    void Load();
    FriendList::Error AddFriendByName(const std::string& playerName, AB::Entities::FriendRelation relation);
    FriendList::Error Remove(const std::string& playerName);
    FriendList::Error ChangeNickname(const std::string& currentName, const std::string& newName);
};

}
