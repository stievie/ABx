#pragma once

#include "Vector3.h"
#include <AB/ProtocolCodes.h>

namespace Game {

class Creature;

namespace Components {

class MoveComp
{
private:
    Creature& owner_;
    Math::Vector3 oldPosition_;
public:
    static constexpr float BaseSpeed = 150.0f;
    MoveComp(Creature& owner) :
        owner_(owner),
        moveDir_(AB::GameProtocol::MoveDirectionNone),
        turnDir_(AB::GameProtocol::TurnDirectionNone),
        oldPosition_(Math::Vector3::Zero),
        moved_(false),
        turned_(false),
        directionSet_(false)
    { }
    ~MoveComp() = default;

    /// Move in direction of rotation
    bool Move(float speed, const Math::Vector3& amount);
    /// Move to moveDir_
    bool MoveTo(uint32_t timeElapsed);
    void Turn(float angle);
    /// Turn to turnDir_
    void TurnTo(uint32_t timeElapsed);
    void SetDirection(float worldAngle);

    const Math::Vector3& GetOldPosition() const
    {
        return oldPosition_;
    }

    uint32_t moveDir_;
    uint32_t turnDir_;
    bool moved_;
    bool turned_;
    /// Manual direction set
    bool directionSet_;
};

}
}
