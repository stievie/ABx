#include "stdafx.h"
#include "Queue.h"
#include "Subsystems.h"
#include "DataClient.h"
#include <AB/Entities/Game.h>
#include <algorithm>
#include <AB/Entities/Character.h>
#include "MessageClient.h"

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

std::vector<std::string> Queue::MakeRandomTeam()
{
    std::vector<std::string> result;
    return result;
}

void Queue::Update(uint32_t)
{
    if (EnoughPlayers())
    {

    }
}
