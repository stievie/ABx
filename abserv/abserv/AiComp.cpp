#include "stdafx.h"
#include "AiComp.h"
#include "Npc.h"

namespace Game {
namespace Components {

AiComp::AiComp(Npc& owner) :
    owner_(owner),
    agent_(owner)
{ }

void AiComp::Update(uint32_t timeElapsed)
{
    if (!owner_.IsDead())
        agent_.Update(timeElapsed);
}

}
}
