#pragma once

#include "NetworkMessage.h"
#include "IdGenerator.h"

namespace Game {

class Player;
class PartyChatChannel;

class Party : public std::enable_shared_from_this<Party>
{
private:
    static Utils::IdGenerator<uint32_t> partyIds_;
    std::weak_ptr<Player> leader_;
    std::vector<std::weak_ptr<Player>> members_;
    /// Used when forming a group. If the player accepts it is added to the members.
    std::vector<std::weak_ptr<Player>> invited_;
    std::shared_ptr<PartyChatChannel> chatChannel_;
    /// Depends on the map
    uint32_t maxMembers_;
    uint32_t GetNewId()
    {
        return partyIds_.Next();
    }
public:
    explicit Party(std::shared_ptr<Player> leader);
    Party() = delete;
    // non-copyable
    Party(const Party&) = delete;
    Party& operator=(const Party&) = delete;

    ~Party();

    bool Add(std::shared_ptr<Player> player);
    bool Remove(std::shared_ptr<Player> player);
    bool Invite(std::shared_ptr<Player> player);
    bool RemoveInvite(std::shared_ptr<Player> player);
    void ClearInvites();

    void WriteToMembers(const Net::NetworkMessage& message);

    void SetPartySize(uint32_t size);
    size_t GetMemberCount() const
    {
        return members_.size();
    }
    const std::vector<std::weak_ptr<Player>>& GetMembers() const
    {
        return members_;
    }
    bool IsMember(std::shared_ptr<Player> player) const;
    bool IsInvited(std::shared_ptr<Player> player) const;
    bool IsLeader(const Player* const player);
    std::shared_ptr<Player> GetLeader() const
    {
        return leader_.lock();
    }
    /// Tells all members to change the instance. The client will disconnect and reconnect to enter the instance.
    void ChangeInstance(const std::string& mapUuid);

    uint32_t id_;
};

}
