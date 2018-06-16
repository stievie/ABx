#pragma once

namespace Game {

class Player;
class PartyChatChannel;

class Party : public std::enable_shared_from_this<Party>
{
private:
    std::weak_ptr<Player> leader_;
    std::vector<std::weak_ptr<Player>> members_;
    /// Used when forming a group. If the player accepts it is added to the members.
    std::vector<std::weak_ptr<Player>> invited_;
    std::vector<std::weak_ptr<Player>> requesters_;
    std::shared_ptr<PartyChatChannel> chatChannel_;
    /// Depends on the map
    uint32_t maxMembers_;
    static uint32_t partyIds_;
    uint32_t GetNewId()
    {
        if (partyIds_ >= std::numeric_limits<uint32_t>::max())
            partyIds_ = 0;
        return ++partyIds_;
    }
public:
    Party(std::shared_ptr<Player> leader);
    ~Party() = default;

    bool Add(std::shared_ptr<Player> player);
    bool Remove(std::shared_ptr<Player> player);
    bool Invite(std::shared_ptr<Player> player);
    bool RemoveInvite(std::shared_ptr<Player> player);
    bool Request(std::shared_ptr<Player> player);
    bool RemoveRequest(std::shared_ptr<Player> player);

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
    bool IsRequester(std::shared_ptr<Player> player) const;

    uint32_t id_;
};

}
