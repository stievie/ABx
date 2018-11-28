#pragma once

#include "Vector3.h"
#include <AB/ProtocolCodes.h>

namespace Game {

class Actor;

namespace Components {

class MoveComp
{
private:
    Actor& owner_;
    Math::Vector3 oldPosition_;
    float speedFactor_;
public:
    static constexpr float BaseSpeed = 150.0f;
    explicit MoveComp(Actor& owner) :
        owner_(owner),
        moveDir_(AB::GameProtocol::MoveDirectionNone),
        turnDir_(AB::GameProtocol::TurnDirectionNone),
        oldPosition_(Math::Vector3::Zero),
        speedFactor_(1.0f),
        moved_(false),
        turned_(false),
        directionSet_(false),
        newAngle_(false),
        speedDirty_(false)
    { }
    ~MoveComp() = default;

    void Update(uint32_t timeElapsed);
    bool SetPosition(const Math::Vector3& pos);
    void HeadTo(const Math::Vector3& pos);
    /// Move in direction of rotation
    bool Move(float speed, const Math::Vector3& amount);
    /// Move to moveDir_
    bool MoveTo(uint32_t timeElapsed);
    void Turn(float angle);
    /// Turn to turnDir_
    void TurnTo(uint32_t timeElapsed);
    void SetDirection(float worldAngle);
    float GetSpeedFactor() const
    {
        return speedFactor_;
    }
    void SetSpeedFactor(float value)
    {
        if (speedFactor_ != value)
        {
            speedFactor_ = value;
            speedDirty_ = true;
        }
    }
    float GetSpeed() const
    {
        return BaseSpeed * speedFactor_;
    }

    const Math::Vector3& GetOldPosition() const
    {
        return oldPosition_;
    }
    void Write(Net::NetworkMessage& message);

    uint32_t moveDir_;
    uint32_t turnDir_;
    bool moved_;
    bool turned_;
    bool speedDirty_;
    /// Manual direction set
    bool directionSet_;
    bool newAngle_;
};

}
}
