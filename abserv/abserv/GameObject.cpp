#include "stdafx.h"
#include "GameObject.h"
#include "Game.h"

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
    boundingBox_(-2.0f, 2.0f),
    octant_(nullptr)
{
    id_ = GetNewId();
}

GameObject::~GameObject()
{
    RemoveFromOctree();
}

bool GameObject::Serialize(IO::PropWriteStream& stream)
{
    stream.Write<uint8_t>(GetType());
    stream.WriteString(GetName());
    return true;
}

void GameObject::AddToOctree()
{
    if (auto g = game_.lock())
    {
        Math::Octree* octree = g->map_->octree_.get();
        octree->InsertObject(this);
    }
}

void GameObject::RemoveFromOctree()
{
    if (octant_)
    {
        Math::Octree* octree = octant_->GetRoot();
        octree->RemoveObject(this);
    }
}

}
