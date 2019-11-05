#include "stdafx.h"
#include "AiAgent.h"
#include "Npc.h"

namespace AI {

AiAgent::AiAgent(Game::Npc& npc) :
    Agent(npc.id_),
    npc_(npc)
{ }

AiAgent::~AiAgent() = default;

}
