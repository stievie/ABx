#include "stdafx.h"
#include "GameObject.h"
#include "MathUtils.h"

#include <Urho3D/DebugNew.h>

GameObject::GameObject(Context* context) :
    LogicComponent(context),
    objectType_(ObjectTypeStatic),
    creatureState_(AB::GameProtocol::CreatureStateIdle)
{
}

GameObject::~GameObject()
{
}

void GameObject::SetYRotation(float rad, bool updateYaw)
{
    Quaternion direction;
    float deg = RadToDeg(rad);
    direction.FromAngleAxis(deg, Vector3::UP);
    GetNode()->SetRotation(direction);
}

float GameObject::GetYRotation()
{
    return DegToRad(GetNode()->GetRotation().YawAngle());
}

void GameObject::MoveTo(const Vector3 & newPos)
{
    GetNode()->SetPosition(newPos);
}
