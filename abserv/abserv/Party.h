#pragma once

#include "NetworkMessage.h"
#include "IdGenerator.h"
#include <AB/Entities/Party.h>
#include "Subsystems.h"
#include "DataClient.h"

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
    uint32_t maxMembers_;
    int64_t resignedTick_;
    template<typename E>
    bool UpdateEntity(const E& e)
    {
        IO::DataClient* cli = GetSubsystem<IO::DataClient>();
        return cli->Update(e);
    }
    /// 1-base position
    size_t GetDataPos(Player* player);
    Player* _LuaGetLeader();
public:
    static Utils::IdGenerator<uint32_t> partyIds_;
    static uint32_t GetNewId()
    {
        return partyIds_.Next();
    }
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
    bool Remove(Player* player, bool newParty = true);
    bool Invite(std::shared_ptr<Player> player);
    bool RemoveInvite(std::shared_ptr<Player> player);
    void ClearInvites();

    void Update(uint32_t timeElapsed, Net::NetworkMessage& message);
    void WriteToMembers(const Net::NetworkMessage& message);

    void SetPartySize(size_t size);
    size_t GetMemberCount() const
    {
        return members_.size();
    }
    const std::vector<std::weak_ptr<Player>>& GetMembers() const
    {
        return members_;
    }
    bool IsFull() const { return static_cast<uint32_t>(members_.size()) >= maxMembers_; }
    bool IsMember(Player* player) const;
    bool IsInvited(Player* player) const;
    bool IsLeader(Player* player) const;
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
            auto sm = m.lock();
            if (sm)
                return sm;
        }
        return std::shared_ptr<Player>();
    }
    void KillAll();

    /// Get position of actor in party, 1-based, 0 = not found
    size_t GetPosition(Actor* actor);
    /// Tells all members to change the instance. The client will disconnect and reconnect to enter the instance.
    void ChangeInstance(const std::string& mapUuid);

    uint32_t id_;
    AB::Entities::Party data_;
};

}
