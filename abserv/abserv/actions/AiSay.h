
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
    int ch = 0;
    int quote = 0;

#ifdef _MSC_VER
    if (::sscanf_s(_parameters.c_str(), "%d,%d", &ch, &quote) != 2) {
#else
    if (::sscanf(_parameters.c_str(), "%d,%d", &ch, &quote) != 2) {
#endif
        return ai::TreeNodeStatus::FAILED;
    }
    if (ch < 1 || ch > 6)
        return ai::TreeNodeStatus::FAILED;
    Game::ChatType channel = static_cast<Game::ChatType>(ch);
    if (!npc.SayQuote(channel, quote))
        return ai::TreeNodeStatus::FAILED;
    return ai::TreeNodeStatus::FINISHED;
}

}
