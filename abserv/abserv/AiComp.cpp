#include "stdafx.h"
#include "AiComp.h"

namespace Game {
namespace Components {

AiComp::AiComp(Npc & owner) :
    owner_(owner),
    agent_(owner)
{ }

void AiComp::Update(uint32_t timeElapsed)
{
    agent_.Update(timeElapsed);
}

}
}
