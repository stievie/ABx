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

#include <absmath/Vector3.h>
#include <AB/ProtocolCodes.h>
#include <sa/Noncopyable.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

class Actor;

namespace Components {

class MoveComp
{
    NON_COPYABLE(MoveComp)
    NON_MOVEABLE(MoveComp)
private:
    Actor& owner_;
    float speedFactor_{ 1.0f };
    Math::Vector3 oldPosition_;
    Math::Vector3 safePosition_;
    /// Move to moveDir_
    void UpdateMove(uint32_t timeElapsed);
    /// Turn to turnDir_
    void UpdateTurn(uint32_t timeElapsed);
public:
    enum UpdateFlags : uint32_t
    {
        UpdateFlagMove = 1,
        UpdateFlagTurn = 1 << 1
    };
    MoveComp() = delete;
    explicit MoveComp(Actor& owner) :
        owner_(owner)
    { }
    ~MoveComp() = default;

    inline float GetSpeed(uint32_t timeElapsed, float baseSpeed)
    {
        return (static_cast<float>(timeElapsed) / baseSpeed) * speedFactor_;
    }
    void Update(uint32_t timeElapsed, uint32_t flags);
    bool SetPosition(const Math::Vector3& pos);
    void StickToGround();
    void HeadTo(const Math::Vector3& pos);
    /// Move in direction of rotation
    void Move(float speed, const Math::Vector3& amount);
    void Turn(float angle);
    void SetDirection(float worldAngle);
    float GetSpeedFactor() const
    {
        return speedFactor_;
    }
    void SetSpeedFactor(float value)
    {
        if (Math::Equals(speedFactor_, value))
            return;

        speedFactor_ = value;
        speedDirty_ = true;
    }
    void AddSpeed(float value)
    {
        if (Math::Equals(value, 0.0f))
            return;

        speedFactor_ += value;
        speedDirty_ = true;
    }

    const Math::Vector3& GetOldPosition() const
    {
        return oldPosition_;
    }
    const Math::Vector3& GetSafePosition() const
    {
        return safePosition_;
    }
    bool IsMoving() const { return !velocity_.Equals(Math::Vector3::Zero); }
    void Write(Net::NetworkMessage& message);
    void StoreOldPosition();
    void StoreSafePosition();
    Math::Vector3& CalculateVelocity(uint32_t timeElapsed);

    uint32_t moveDir_{ AB::GameProtocol::MoveDirectionNone };
    uint32_t turnDir_{ AB::GameProtocol::TurnDirectionNone };
    bool turned_{ false };
    bool moved_{ false };
    bool speedDirty_{ false };
    /// Manual direction set
    bool directionSet_{ false };
    bool newAngle_{ false };
    /// Sends a special message to the client to force the client to set the position.
    bool forcePosition_{ false };
    bool autoMove_{ false };
    /// Velocity in Units/s.
    Math::Vector3 velocity_;
};

}
}
