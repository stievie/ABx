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

#include "Group.h"
#include <AB/Entities/Party.h>
#include <abscommon/DataClient.h>
#include <abscommon/NetworkMessage.h>
#include <abscommon/Subsystems.h>
#include <abscommon/Variant.h>
#include <abshared/Mechanic.h>
#include <kaguya/kaguya.hpp>
#include <sa/IdGenerator.h>
#include <sa/Iteration.h>
#include <sa/Noncopyable.h>

namespace Game {

class PartyChatChannel;
class Player;

class Party final : public Group, public std::enable_shared_from_this<Party>
{
    NON_COPYABLE(Party)
private:
    /// Used when forming a group. If the player accepts it is added to the members.
    std::vector<std::weak_ptr<Player>> invited_;
    std::shared_ptr<PartyChatChannel> chatChannel_;
    /// Depends on the map
    uint32_t maxMembers_{ 1 };
    int64_t defeatedTick_{ 0 };
    bool defeated_{ false };
    Utils::VariantMap variables_;
    template<typename E>
    bool UpdateEntity(const E& e)
    {
        IO::DataClient* cli = GetSubsystem<IO::DataClient>();
        return cli->Update(e);
    }
    /// 1-base position
    size_t GetDataPos(const Player& player);
    std::string _LuaGetVarString(const std::string& name);
    void _LuaSetVarString(const std::string& name, const std::string& value);
    float _LuaGetVarNumber(const std::string& name);
    void _LuaSetVarNumber(const std::string& name, float value);
public:
    static void RegisterLua(kaguya::State& state);

    Party();
    ~Party();

    /// Append player
    bool AddPlayer(std::shared_ptr<Player> player);
    /// Insert at the position in data_
    bool SetPlayer(std::shared_ptr<Player> player);
    bool RemovePlayer(Player& player, bool newParty = true);
    bool Invite(std::shared_ptr<Player> player);
    bool RemoveInvite(std::shared_ptr<Player> player);
    /// Clear all invites
    void ClearInvites();
    const std::vector<std::weak_ptr<Actor>>& GetMembers() const { return members_; }
    Player* GetRandomPlayer() const;
    Player* GetRandomPlayerInRange(const Actor* actor, Ranges range) const;

    void Update(uint32_t timeElapsed, Net::NetworkMessage& message);
    void WriteToMembers(const Net::NetworkMessage& message);

    void SetPartySize(size_t size);
    inline size_t GetValidPlayerCount() const;
    inline size_t GetMemberCount() const { return members_.size(); }
    void VisitPlayers(const std::function<Iteration(Player& current)>& callback) const;
    bool IsFull() const { return static_cast<uint32_t>(members_.size()) >= maxMembers_; }
    bool IsMember(const Player& player) const;
    bool IsInvited(const Player& player) const;
    bool IsLeader(const Player& player) const;
    std::shared_ptr<Player> GetAnyPlayer() const;
    void Defeat();
    bool IsDefeated() const { return defeated_; }
    /// Bring all players back to their last outpost
    void TeleportBack();

    /// Get position of actor in party, 1-based, 0 = not found
    size_t GetPosition(const Actor* actor) const;
    /// Tells all members to change the instance. The client will disconnect and reconnect to enter the instance.
    void ChangeInstance(const std::string& mapUuid);
    void ChangeServerInstance(const std::string& serverUuid, const std::string& mapUuid, const std::string& instanceUuid);
    void NotifyPlayersQueued();
    void NotifyPlayersUnqueued();

    const Utils::Variant& GetVar(const std::string& name) const;
    void SetVar(const std::string& name, const Utils::Variant& val);

    AB::Entities::Party data_;
    /// Get the ID of the game the members are in
    uint32_t gameId_{ 0 };
};

}
