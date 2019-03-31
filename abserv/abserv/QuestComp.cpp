#include "stdafx.h"
#include "QuestComp.h"
#include "Player.h"

namespace Game {
namespace Components {

void QuestComp::Update(uint32_t timeElapsed)
{
    // TODO:
    for (const auto& q : quests_)
    {
        q->Update(timeElapsed);
    }
}

void QuestComp::Write(Net::NetworkMessage& /* message */)
{
    // TODO:
}

}
}
