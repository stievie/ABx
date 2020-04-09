/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <AB/ProtocolCodes.h>
#include <abscommon/Utils.h>
#include <sa/Noncopyable.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class GameObject;

namespace Components {

class StateComp
{
    NON_COPYABLE(StateComp)
    NON_MOVEABLE(StateComp)
private:
    GameObject& owner_;
    AB::GameProtocol::CreatureState currentState_{ AB::GameProtocol::CreatureState::Idle };
    AB::GameProtocol::CreatureState newState_{ AB::GameProtocol::CreatureState::Idle };
    int64_t lastStateChange_;
    int64_t knockdownEndTime_{ 0 };
    bool groupMaskChanged_{ false };
public:
    StateComp() = delete;
    explicit StateComp(GameObject& owner) :
        owner_(owner),
        lastStateChange_(Utils::Tick())
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
    bool IsDead() const { return newState_ == AB::GameProtocol::CreatureState::Dead; }
    bool IsKnockedDown() const { return newState_ == AB::GameProtocol::CreatureState::KnockedDown; }
    bool IsMoving() const { return newState_ == AB::GameProtocol::CreatureState::Moving; }
    bool KnockDown(uint32_t time);
    void Apply();
    /// Reset to idle state
    void Reset();
    void GroupMaskChanged();
    void Update(uint32_t timeElapsed);
    void Write(Net::NetworkMessage& message);
};

}
}

