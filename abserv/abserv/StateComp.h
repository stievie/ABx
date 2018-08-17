#pragma once

#include <AB/ProtocolCodes.h>

namespace Game {

class GameObject;

namespace Components {

class StateComp
{
private:
    GameObject& owner_;
    AB::GameProtocol::CreatureState currentState_;
    AB::GameProtocol::CreatureState newState_;
    int64_t lastStateChange_;
public:
    StateComp(GameObject& owner) :
        owner_(owner),
        newState_(AB::GameProtocol::CreatureStateIdle),
        currentState_(AB::GameProtocol::CreatureStateIdle),
        lastStateChange_(Utils::AbTick())
    { }
    ~StateComp() = default;

    void SetState(AB::GameProtocol::CreatureState state, bool apply = false);
    AB::GameProtocol::CreatureState GetState() const
    {
        return currentState_;
    }
    bool IsStateChanged() const
    {
        return newState_ != currentState_;
    }
    void Apply();
    void Update(uint32_t timeElapsed);
};

}
}

