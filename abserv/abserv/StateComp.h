#pragma once

#include <AB/ProtocolCodes.h>
#include "Utils.h"

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
    int64_t knockdownEndTime_;
public:
    StateComp() = delete;
    explicit StateComp(GameObject& owner) :
        owner_(owner),
        currentState_(AB::GameProtocol::CreatureStateIdle),
        newState_(AB::GameProtocol::CreatureStateIdle),
        lastStateChange_(Utils::Tick()),
        knockdownEndTime_(0)
    { }
    // non-copyable
    StateComp(const StateComp&) = delete;
    StateComp& operator=(const StateComp&) = delete;
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
    bool IsDead() const { return newState_ == AB::GameProtocol::CreatureStateDead; }
    bool IsKnockedDown() const { return newState_ == AB::GameProtocol::CreatureStateKnockedDown; }
    bool KnockDown(uint32_t time);
    void Apply();
    /// Reset to idle state
    void Reset();
    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message);
};

}
}

