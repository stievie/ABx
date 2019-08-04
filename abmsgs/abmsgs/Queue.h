#pragma once

#include <AB/Entities/Profession.h>

struct QueueEntry
{
    std::string uuid;
    bool isParty;
    AB::Entities::ProfessionPosition position;
};

class Queue
{
private:
    static AB::Entities::ProfessionPosition GetPlayerPosition(const std::string& uuid);
    std::string uuid_;
    std::string mapUuid_;
    std::vector<QueueEntry> entires_;
    unsigned partySize_{ 0 };
    unsigned partyCount_{ 0 };
    /// If true, choose partyCount_ * partySize_ players and put them into pertyCount_ parties.
    /// If false, the party leader is in the parties_ list and if there are partyCount_ parties they can enter a match.
    bool randomParty_{ false };
    std::vector<std::string> MakeRandomTeam();
public:
    explicit Queue(const std::string& mapUuid);
    bool Load();
    void Add(const std::string& uuid);
    void Remove(const std::string& uuid);

    size_t Count() const { return entires_.size(); }
    bool EnoughPlayers() const
    {
        if (randomParty_)
            return Count() >= partySize_ * partyCount_;
        return Count() >= partyCount_;
    }
    void Update(uint32_t timeElapsed);
};
