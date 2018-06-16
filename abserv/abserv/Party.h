#pragma once

namespace Game {

class Player;

class Party : public std::enable_shared_from_this<Party>
{
private:
    std::weak_ptr<Player> leader_;
    std::vector<std::weak_ptr<Player>> members_;
    /// Used when forming a group. If the player accepts it is added to the members.
    std::vector<std::weak_ptr<Player>> invited_;
    std::vector<std::weak_ptr<Player>> requestees_;
    /// Depends on the map
    uint32_t maxMembers_;
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
    bool IsRequestee(std::shared_ptr<Player> player) const;

};

}
