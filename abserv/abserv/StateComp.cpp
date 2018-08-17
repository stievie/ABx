#include "stdafx.h"
#include "StateComp.h"
#include "GameObject.h"

namespace Game {
namespace Components {

void StateComp::SetState(AB::GameProtocol::CreatureState state, bool apply /* = false */)
{
    if (state != currentState_)
    {
        newState_ = state;
        if (apply)
            Apply();
    }
}

void StateComp::Apply()
{
    lastStateChange_ = Utils::AbTick();
    currentState_ = newState_;
}

void StateComp::Update(uint32_t)
{
    if (currentState_ == AB::GameProtocol::CreatureStateEmoteCry &&
        lastStateChange_ + 10000 < Utils::AbTick())
    {
        // Reset some emotes after 10 seconds
        SetState(AB::GameProtocol::CreatureStateIdle);
    }
}

}
}