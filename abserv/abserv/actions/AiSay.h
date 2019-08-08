
#pragma once

#include "../AiTask.h"
#include "../Npc.h"
#include "StringUtils.h"
#include "../Chat.h"

namespace AI {

AI_TASK(Say)
{
    (void)deltaMillis;
    Game::Npc& npc = chr.GetNpc();
    // channel;message
    std::vector<std::string> params = Utils::Split(_parameters, ";");
    if (params.size() != 2)
        return ai::TreeNodeStatus::FAILED;
    Game::ChatType channel = Game::ChatType::None;
    const std::string& ch = params[0];
    if (ch.compare("map") == 0)
        channel = Game::ChatType::Map;
    else if (ch.compare("party") == 0)
        channel == Game::ChatType::Party;
    else if (ch.compare("allies") == 0)
        channel == Game::ChatType::Allies;
    // All other channels don't make sense for an NPC, so fail
    if (channel == Game::ChatType::None)
        return ai::TreeNodeStatus::FAILED;
    npc.Say(channel, params[1]);
    return ai::TreeNodeStatus::FINISHED;
}

}
