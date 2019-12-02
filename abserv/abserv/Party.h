#pragma once

#include "DataClient.h"
#include "Group.h"
#include "Mechanic.h"
#include "NetworkMessage.h"
#include "Subsystems.h"
#include "Variant.h"
#include <AB/Entities/Party.h>
#include <kaguya/kaguya.hpp>
#include <sa/IdGenerator.h>
#include <sa/Iteration.h>

namespace Game {

class Player;
class Actor;
class PartyChatChannel;

class Party : public std::enable_shared_from_this<Party>
{
private:
    std::vector<std::weak_ptr<Player>> members_;
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
    Player* _LuaGetLeader();
    std::string _LuaGetVarString(const std::string& name);
    void _LuaSetVarString(const std::string& name, const std::string& value);
    float _LuaGetVarNumber(const std::string& name);
    void _LuaSetVarNumber(const std::string& name, float value);
    Actor* _LuaGetMember(int index);
    int _LuaGetMemberCount();
public:
    static void RegisterLua(kaguya::State& state);

    Party();
    // non-copyable
    Party(const Party&) = delete;
    Party& operator=(const Party&) = delete;

    ~Party();

    /// Append player
    bool Add(std::shared_ptr<Player> player);
    /// Insert at the position in data_
    bool Set(std::shared_ptr<Player> player);
    bool Remove(Player& player, bool newParty = true);
    bool Invite(std::shared_ptr<Player> player);
    bool RemoveInvite(std::shared_ptr<Player> player);
    /// Clear all invites
    void ClearInvites();

    void Update(uint32_t timeElapsed, Net::NetworkMessage& message);
    void WriteToMembers(const Net::NetworkMessage& message);

    void SetPartySize(size_t size);
    inline size_t GetValidMemberCount() const;
    inline size_t GetMemberCount() const { return members_.size(); }
    const std::vector<std::weak_ptr<Player>>& GetMembers() const
    {
        return members_;
    }
    /// Iteration callback(Player& player)
    template<typename Callback>
    inline void VisitMembers(const Callback& callback) const
    {
        for (auto& wm : members_)
        {
            if (auto sm = wm.lock())
                if (callback(*sm) != Iteration::Continue)
                    break;
        }
    }
    bool IsFull() const { return static_cast<uint32_t>(members_.size()) >= maxMembers_; }
    bool IsMember(const Player& player) const;
    bool IsInvited(const Player& player) const;
    bool IsLeader(const Player& player) const;
    std::shared_ptr<Player> GetLeader() const
    {
        if (members_.size() == 0)
            return std::shared_ptr<Player>();
        return members_[0].lock();
    }
    std::shared_ptr<Player> GetAnyMember() const
    {
        if (members_.size() == 0)
            return std::shared_ptr<Player>();
        for (const auto& m : members_)
        {
            if (auto sm = m.lock())
                return sm;
        }
        return std::shared_ptr<Player>();
    }
    /// Return a random Player of this Party
    Player* GetRandomPlayer() const;
    Player* GetRandomPlayerInRange(const Actor* actor, Ranges range) const;
    void KillAll();
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

    uint32_t id_;
    AB::Entities::Party data_;
    /// Get the ID of the game the members are in
    uint32_t gameId_{ 0 };
};

}
