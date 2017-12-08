#include "stdafx.h"
#include "GameObject.h"

#include "DebugNew.h"

namespace Game {

uint32_t GameObject::objectIds_ = 0;

void GameObject::RegisterLua(kaguya::State& state)
{
    state["GameObject"].setClass(kaguya::UserdataMetatable<GameObject>()
        .addFunction("GetId", &GameObject::GetId)
        .addFunction("GetGame", &GameObject::GetGame)
        .addFunction("GetName", &GameObject::GetName)
    );
}

GameObject::GameObject() :
    rotation_(0.0f)
{
    id_ = GetNewId();
}

bool GameObject::Serialize(IO::PropWriteStream& stream)
{
    stream.Write<uint8_t>(GetType());
    stream.WriteString(GetName());
    return true;
}

}
