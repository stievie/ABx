#include "stdafx.h"
#include "StateComp.h"
#include "GameObject.h"

namespace Game {
namespace Components {

void StateComp::SetState(AB::GameProtocol::CreatureState state, bool apply /* = false */)
{
    if (state != newState_)
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

void StateComp::Write(Net::NetworkMessage& message)
{
    if (IsStateChanged())
    {
        Apply();
#ifdef DEBUG_GAME
        LOG_DEBUG << "New state of " << owner_.id_ << ":" << (int)GetState() << std::endl;
#endif
        message.AddByte(AB::GameProtocol::GameObjectStateChange);
        message.Add<uint32_t>(owner_.id_);
        message.AddByte(GetState());
    }
}

}
}