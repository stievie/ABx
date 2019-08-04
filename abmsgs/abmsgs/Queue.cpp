#include "stdafx.h"
#include "Queue.h"
#include "Subsystems.h"
#include "DataClient.h"
#include <AB/Entities/Game.h>
#include <algorithm>

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

    return true;
}

void Queue::Add(const std::string& partyUuid)
{
    parties_.push_back(partyUuid);
}

void Queue::Remove(const std::string& partyUuid)
{
    auto it = std::find_if(parties_.begin(), parties_.end(), [&](const auto& current) {
        return current.compare(partyUuid) == 0;
    });
    if (it != parties_.end())
        parties_.erase(it);
}
