#include "stdafx.h"
#include "GameObject.h"
#include "MathUtils.h"

GameObject::GameObject(Context* context) :
    LogicComponent(context),
    objectType_(ObjectTypeStatic),
    creatureState_(AB::GameProtocol::CreatureStateIdle)
{
}

GameObject::~GameObject()
{
}

void GameObject::SetYRotation(float rad)
{
    Quaternion direction;
    float deg = RadToDeg(rad);
    direction.FromAngleAxis(deg, Vector3(0.0f, 1.0f, 0.0f));
    GetNode()->SetRotation(direction);
}

float GameObject::GetYRotation()
{
    return DegToRad(GetNode()->GetRotation().YawAngle());
}
