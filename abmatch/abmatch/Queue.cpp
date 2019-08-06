#include "stdafx.h"
#include "Queue.h"
#include "Subsystems.h"
#include "DataClient.h"
#include <AB/Entities/Game.h>
#include <algorithm>
#include <AB/Entities/Character.h>
#include "MessageClient.h"
#include <AB/Entities/Account.h>
#include "Transaction.h"

AB::Entities::ProfessionPosition Queue::GetPlayerPosition(const std::string& uuid)
{
    auto* dataclient = GetSubsystem<IO::DataClient>();
    AB::Entities::Character c;
    c.uuid = uuid;
    if (!dataclient->Read(c))
    {
        LOG_ERROR << "Error reading character " << uuid << std::endl;
        return AB::Entities::ProfessionPosition::None;
    }
    AB::Entities::Profession prof;
    prof.uuid = c.professionUuid;
    if (!dataclient->Read(prof))
    {
        LOG_ERROR << "Error reading profession " << c.professionUuid << std::endl;
        return AB::Entities::ProfessionPosition::None;
    }

    return prof.position;
}

Queue::Queue(const std::string& mapUuid) :
    mapUuid_(mapUuid)
{
    const uuids::uuid guid = uuids::uuid_system_generator{}();
    uuid_ = guid.to_string();
}

bool Queue::Load()
{
    auto* dataclient = GetSubsystem<IO::DataClient>();
    AB::Entities::Game game;
    game.uuid = mapUuid_;
    if (!dataclient->Read(game))
    {
        LOG_ERROR << "Error reading map with UUID " << mapUuid_ << std::endl;
        return false;
    }
    partySize_ = game.partySize;
    if (partySize_ == 0)
    {
        LOG_ERROR << "Party size can not be 0" << std::endl;
        return false;
    }
    partyCount_ = game.partyCount;
    if (partyCount_ == 0)
    {
        LOG_ERROR << "Party count can not be 0" << std::endl;
        return false;
    }
    randomParty_ = game.randomParty;
    return true;
}

void Queue::Add(const std::string& uuid)
{
    entires_.push_back({
       uuid,
       !randomParty_,
       randomParty_ ? GetPlayerPosition(uuid) : AB::Entities::ProfessionPosition::None
   });

    auto* client = GetSubsystem<Net::MessageClient>();
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::PlayerAddedToQueue;

    IO::PropWriteStream stream;
    stream.WriteString(uuid);
    stream.WriteString(uuid_);
    msg.SetPropStream(stream);
    client->Write(msg);
}

void Queue::Remove(const std::string& uuid)
{
    auto it = std::find_if(entires_.begin(), entires_.end(), [&](const QueueEntry& current) {
        return current.uuid.compare(uuid) == 0;
    });
    if (it != entires_.end())
    {
        entires_.erase(it);

        auto* client = GetSubsystem<Net::MessageClient>();
        Net::MessageMsg msg;
        msg.type_ = Net::MessageType::PlayerRemovedFromQueue;

        IO::PropWriteStream stream;
        stream.WriteString(uuid);
        stream.WriteString(uuid_);
        msg.SetPropStream(stream);
        client->Write(msg);
    }
}

std::string Queue::FindServerForMatch(const MatchTeams& teams)
{
    auto* dataclient = GetSubsystem<IO::DataClient>();

    std::vector<std::string> servers;
    std::map<std::string, unsigned> sorting;
    for (const auto& t : teams.teams)
    {
        for (const auto& m : t.members)
        {
            AB::Entities::Character ch;
            ch.uuid = m;
            if (!dataclient->Read(ch))
                continue;
            AB::Entities::Account acc;
            acc.uuid = ch.accountUuid;
            if (!dataclient->Read(acc))
                continue;

            servers.push_back(acc.currentServerUuid);
            sorting[acc.currentServerUuid] += 1;
        }
    }
    if (servers.size() == 0)
    {
        LOG_ERROR << "No server found" << std::endl;
        return "";
    }

    // Make server with most ppl on it the first
    std::sort(servers.begin(), servers.end(), [&sorting](const std::string& i1, const std::string& i2) {
        const auto& p1 = sorting[i1];
        const auto& p2 = sorting[i2];

        return p1 > p2;
    });
    return servers[0];
}

void Queue::SendEnterMessage(const MatchTeams& teams)
{
    auto* client = GetSubsystem<Net::MessageClient>();
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::TeamsEnterMatch;

    IO::PropWriteStream stream;
    // This server will host the game
    std::string server = FindServerForMatch(teams);
    if (server.empty())
    {
        // This sould really not happen
        LOG_ERROR << "Didn not find any suitable server" << std::endl;
        return;
    }
    stream.WriteString(teams.queueUuid);
    stream.WriteString(server);
    stream.WriteString(teams.mapUuid);
    stream.Write<uint8_t>(static_cast<uint8_t>(teams.teams.size()));
    for (const auto& team : teams.teams)
    {
        stream.Write<uint8_t>(static_cast<uint8_t>(team.members.size()));
        for (const auto& member : team.members)
        {
            stream.WriteString(member);
        }
    }
    msg.SetPropStream(stream);
    client->Write(msg);
}

bool Queue::MakeRandomTeams(MatchTeams& teams)
{
    (void)teams;
    return false;
}

bool Queue::MakeTeams(MatchTeams& teams)
{
    Utils::Transaction transction(entires_);

    // Not a random team, so only the party leader queued for a game
    for (unsigned i = 0; i < partyCount_; ++i)
    {
        if (entires_.empty())
        {
            // Shouldn't happen because we check first if there are enough players
            LOG_ERROR << "Not enough players" << std::endl;
            return false;
        }
        Team team;
        QueueEntry& e = entires_.front();
        team.members.push_back(e.uuid);
        entires_.pop_front();
        teams.teams.push_back(team);
    }
    transction.Commit();
    return true;
}

void Queue::Update(uint32_t)
{
    if (EnoughPlayers())
    {
        MatchTeams teams;
        teams.mapUuid = mapUuid_;
        teams.queueUuid = uuid_;
        if (randomParty_)
        {
            if (MakeRandomTeams(teams))
            {
                SendEnterMessage(teams);
            }
        }
        else
        {
            if (MakeTeams(teams))
            {
                SendEnterMessage(teams);
            }
        }
    }
}
