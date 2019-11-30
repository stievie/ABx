#include "stdafx.h"
#include "Group.h"
#include "Actor.h"
#include "Npc.h"

namespace Game {

sa::IdGenerator<uint32_t> Group::groupIds_;

Group::Group(uint32_t id) :
    id_(id)
{ }

Group::Group(uint32_t id, TeamColor color) :
    color_(color),
    id_(id)
{ }

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
    members_.push_back(actor);
    if (Is<Npc>(*actor))
        To<Npc>(*actor).SetGroupId(id_);
}

}
