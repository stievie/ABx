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

bool StateComp::KnockDown(uint32_t time)
{
    if (IsKnockedDown())
        return false;
    knockdownEndTime_ = Utils::Tick() + time;
    SetState(AB::GameProtocol::CreatureStateKnockedDown);
    return true;
}

void StateComp::Apply()
{
    lastStateChange_ = Utils::Tick();
    currentState_ = newState_;
}

void StateComp::Reset()
{
    SetState(AB::GameProtocol::CreatureStateIdle);
}

void StateComp::Update(uint32_t)
{
    if (IsKnockedDown())
    {
        if (knockdownEndTime_ <= Utils::Tick())
        {
            SetState(AB::GameProtocol::CreatureStateIdle);
        }
        return;
    }
    if ((currentState_ > AB::GameProtocol::CreatureStateEmoteStart &&
        currentState_ < AB::GameProtocol::CreatureStateEmoteEnd)
        && lastStateChange_ + 4000 < Utils::Tick())
    {
        // Reset some emotes after 4 seconds
        SetState(AB::GameProtocol::CreatureStateIdle);
    }
}

void StateComp::Write(Net::NetworkMessage& message)
{
    if (IsStateChanged())
    {
        Apply();
#ifdef DEBUG_GAME
        LOG_DEBUG << "New state of " << owner_ << ": " << (int)GetState() << std::endl;
#endif
        message.AddByte(AB::GameProtocol::GameObjectStateChange);
        message.Add<uint32_t>(owner_.id_);
        message.AddByte(GetState());
    }
}

}
}
