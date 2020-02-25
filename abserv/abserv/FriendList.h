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

#include <AB/Entities/FriendList.h>
#include <sa/Iteration.h>
#include <AB/Entities/Character.h>

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
    FriendList::Error AddFriendAccount(const AB::Entities::Character& ch,
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

    bool Exists(const std::string& accountUuid) const;
    bool IsFriend(const std::string& accountUuid) const;
    bool IsIgnored(const std::string& accountUuid) const;
    bool IsIgnoredByName(const std::string& name) const;
    bool GetFriendByName(const std::string& name, AB::Entities::Friend& f);
    bool GetFriendByAccount(const std::string& accountUuid, AB::Entities::Friend& f);
    template <typename Callback>
    void VisitAll(Callback&& callback) const
    {
        for (auto& f : data_.friends)
            if (callback(f) != Iteration::Continue)
                break;
    }
    size_t Count() const { return data_.friends.size(); }

    AB::Entities::FriendList data_;
};

}
