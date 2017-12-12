#include "stdafx.h"
#include "GameObject.h"


GameObject::GameObject(Context* context) :
    LogicComponent(context),
    objectType_(ObjectTypeStatic)
{
}


GameObject::~GameObject()
{
}
