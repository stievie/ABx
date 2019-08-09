#pragma once

#include <AB/Entities/Profession.h>
#include <deque>
#include <optional>
#include <functional>

struct QueueEntry
{
    std::string uuid;
    AB::Entities::ProfessionPosition position;
};

/// All members of a team (random), or just the leader of a party (which must be on the same server)
struct Team
{
    std::vector<std::string> members;
};

/// The Teams entering the same match
typedef std::vector<Team> MatchTeams;

class Queue
{
private:
    static AB::Entities::ProfessionPosition GetPlayerPosition(const std::string& uuid);
    static std::string FindServerForMatch(const MatchTeams& teams);
    std::string uuid_;
    std::string mapUuid_;
    std::deque<QueueEntry> entries_;
    unsigned partySize_{ 0 };
    unsigned partyCount_{ 0 };
    /// If true, choose partyCount_ * partySize_ players and put them into pertyCount_ parties.
    /// If false, the party leader is in the parties_ list and if there are partyCount_ parties they can enter a match.
    bool randomParty_{ false };
    /// Make one balanced random team if possible
    bool MakeRandomTeam(Team& team);
    bool MakeRandomTeams(MatchTeams& teams);
    bool MakePartyTeams(MatchTeams& teams);
    /// Make all teams for one match
    bool MakeTeams(MatchTeams& teams);
    /// Get the first player for the position
    std::optional<QueueEntry> GetPlayerByPos(AB::Entities::ProfessionPosition pos);
    bool SendEnterMessage(const MatchTeams& teams);
public:
    explicit Queue(const std::string& mapUuid);
    bool Load();
    void Add(const std::string& uuid);
    void Remove(const std::string& uuid);

    size_t Count() const { return entries_.size(); }
    /// Returns true if there are enough players to create one match
    bool EnoughPlayers() const
    {
        if (randomParty_)
            return Count() >= static_cast<size_t>(partySize_) * static_cast<size_t>(partyCount_);
        return Count() >= partyCount_;
    }
    void Update(uint32_t timeElapsed, const std::function<void(const std::string& playerUuid)>& callback);
};
