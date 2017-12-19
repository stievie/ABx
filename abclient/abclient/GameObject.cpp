#include "stdafx.h"
#include "GameObject.h"


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
    float deg = -rad * (180.0f / (float)M_PI);
    direction.FromAngleAxis(deg, Vector3(0.0f, 1.0f, 0.0f));
    GetNode()->SetRotation(direction);
}
