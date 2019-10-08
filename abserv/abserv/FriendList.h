#pragma once

#include <AB/Entities/FriendList.h>
#include <sa/Iteration.h>

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
    FriendList::Error AddFriendAccount(const std::string& accountUuid, const std::string& name,
        AB::Entities::FriendRelation relation);
    static void InvalidateList(const std::string& uuid);
public:
    FriendList(const std::string accountUuid) :
        accountUuid_(accountUuid),
        data_{}
    { }
    ~FriendList() = default;
    FriendList(const FriendList&) = delete;
    FriendList& operator=(const FriendList&) = delete;

    void Load();
    void Save();
    FriendList::Error AddFriendByName(const std::string& playerName, AB::Entities::FriendRelation relation);
    FriendList::Error ChangeNickname(const std::string& accountUuid, const std::string& newName);
    FriendList::Error Remove(const std::string& accountUuid);

    bool Exists(const std::string& accountUuid);
    bool IsFriend(const std::string& accountUuid);
    bool IsIgnored(const std::string& accountUuid);
    bool IsIgnoredByName(const std::string& name);
    bool GetFriendByName(const std::string& name, AB::Entities::Friend& f);
    bool GetFriendByAccount(const std::string& accountUuid, AB::Entities::Friend& f);
    template <typename Callback>
    void VisitAll(const Callback& callback)
    {
        for (auto& f : data_.friends)
            if (callback(f) != Iteration::Continue)
                break;
    }
    size_t Count() const { return data_.friends.size(); }

    AB::Entities::FriendList data_;
};

}
