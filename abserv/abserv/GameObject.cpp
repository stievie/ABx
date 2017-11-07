#include "stdafx.h"
#include "GameObject.h"

#include "DebugNew.h"

namespace Game {

uint32_t GameObject::objectIds_ = 0;

GameObject::GameObject()
{
    id_ = GetNewId();
}


GameObject::~GameObject()
{
}

}
