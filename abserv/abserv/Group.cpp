#include "stdafx.h"
#include "Group.h"
#include "Actor.h"

namespace Game {

sa::IdGenerator<uint32_t> Group::groupIds_;

void Group::RegisterLua(kaguya::State& state)
{
    state["Group"].setClass(kaguya::UserdataMetatable<Group>()
        .addFunction("GetId", &Group::GetId)
        .addFunction("GetLeader", &Group::GetLeader)
        .addFunction("Add", &Group::_LuaAdd)
        .addFunction("Remove", &Group::_LuaRemove)
        .addFunction("GetColor", &Group::GetColor)
        .addFunction("SetColor", &Group::SetColor)
        .addFunction("IsAlly", &Group::IsAlly)
        .addFunction("IsEnemy", &Group::IsEnemy)
    );
}

Group::Group(uint32_t id) :
    id_(id)
{ }

Group::Group(uint32_t id, TeamColor color) :
    color_(color),
    id_(id)
{ }

void Group::_LuaRemove(Actor* actor)
{
    if (!actor)
        return;
    Remove(actor->id_);
}

void Group::_LuaAdd(Actor* actor)
{
    if (!actor)
        return;
    return Add(actor->GetPtr<Actor>());
}

bool Group::IsEnemy(const Group* other) const
{
    auto* l1 = GetLeader();
    if (!l1)
        return false;
    auto* l2 = other->GetLeader();
    if (!l2)
        return false;
    return l1->IsEnemy(l2);
}

bool Group::IsAlly(const Group* other) const
{
    auto* l1 = GetLeader();
    if (!l1)
        return false;
    auto* l2 = other->GetLeader();
    if (!l2)
        return false;
    return l1->IsAlly(l2);
}

void Group::Remove(uint32_t id)
{
    auto it = std::find_if(members_.begin(), members_.end(), [&](const std::weak_ptr<Actor>& current)
    {
        if (auto c = current.lock())
            return c->id_ == id;
        return false;
    });
    if (it == members_.end())
        return;
    members_.erase(it);
}

void Group::Add(std::shared_ptr<Actor> actor)
{
    const auto it = std::find_if(members_.begin(), members_.end(), [&](const std::weak_ptr<Actor>& current)
    {
        if (auto c = current.lock())
            return c->id_ == actor->id_;
        return false;
    });
    if (it != members_.end())
        return;
    actor->SetGroupId(id_);
    members_.push_back(actor);
}

Actor* Group::GetLeader() const
{
    if (members_.size() == 0)
        return nullptr;
    if (auto l = members_.front().lock())
        return l.get();
    return nullptr;
}

}
