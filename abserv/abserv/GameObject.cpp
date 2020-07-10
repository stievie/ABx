/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "Actor.h"
#include "AreaOfEffect.h"
#include "CollisionComp.h"
#include "ConfigManager.h"
#include "Game.h"
#include "GameObject.h"
#include "Npc.h"
#include "Player.h"
#include "TriggerComp.h"
#include <sa/Assert.h>

namespace Game {

sa::IdGenerator<uint32_t> GameObject::objectIds_;

void GameObject::RegisterLua(kaguya::State& state)
{
    // clang-format off
    state["GameObject"].setClass(kaguya::UserdataMetatable<GameObject>()
        .addFunction("GetId",            &GameObject::GetId)
        .addFunction("GetGame",          &GameObject::_LuaGetGame)
        .addFunction("GetName",          &GameObject::GetName)
        .addFunction("SetName",          &GameObject::SetName)
        .addFunction("GetCollisionMask", &GameObject::GetCollisionMask)
        .addFunction("SetCollisionMask", &GameObject::SetCollisionMask)
        .addFunction("QueryObjects",     &GameObject::_LuaQueryObjects)
        .addFunction("Raycast",          &GameObject::_LuaRaycast)
        .addFunction("RaycastTo",        &GameObject::_LuaRaycastTo)
        .addFunction("SetBoundingBox",   &GameObject::_LuaSetBoundingBox)
        .addFunction("SetBoundingSize",  &GameObject::_LuaSetBoundingSize)
        .addFunction("GetVarString",     &GameObject::_LuaGetVarString)
        .addFunction("SetVarString",     &GameObject::_LuaSetVarString)
        .addFunction("GetVarNumber",     &GameObject::_LuaGetVarNumber)
        .addFunction("SetVarNumber",     &GameObject::_LuaSetVarNumber)
        .addFunction("IsTrigger",        &GameObject::IsTrigger)
        .addFunction("SetTrigger",       &GameObject::SetTrigger)
        .addFunction("GetState",         &GameObject::_LuaGetState)
        .addFunction("SetState",         &GameObject::_LuaSetState)

        .addFunction("SetPosition",      &GameObject::_LuaSetPosition)
        .addFunction("SetRotation",      &GameObject::_LuaSetRotation)
        .addFunction("SetScale",         &GameObject::_LuaSetScale)
        .addFunction("SetScaleSimple",   &GameObject::_LuaSetScaleSimple)
        .addFunction("GetPosition",      &GameObject::_LuaGetPosition)
        .addFunction("GetRotation",      &GameObject::_LuaGetRotation)
        .addFunction("GetScale",         &GameObject::_LuaGetScale)
        .addFunction("GetDistance",      &GameObject::GetDistance)

        // Can return empty if up-cast is not possible
        .addFunction("AsActor",          &GameObject::_LuaAsActor)
        .addFunction("AsNpc",            &GameObject::_LuaAsNpc)
        .addFunction("AsPlayer",         &GameObject::_LuaAsPlayer)
        .addFunction("AsAOE",            &GameObject::_LuaAsAOE)

        .addFunction("IsSelectable",     &GameObject::IsSelectable)
        .addFunction("SetSelectable",    &GameObject::SetSelectable)
        .addFunction("GetActorsInRange", &GameObject::_LuaGetActorsInRange)
        .addFunction("GetClosestActor",  &GameObject::_LuaGetClosestActor)
        .addFunction("IsInRange",        &GameObject::IsInRange)
        .addFunction("IsCloserThan",     &GameObject::IsCloserThan)
        .addFunction("IsObjectInSight",  &GameObject::_LuaIsObjectInSight)
        .addFunction("GetObjectsInside", &GameObject::_LuaGetObjectsInside)
        .addFunction("CallGameEvent",    &GameObject::_LuaCallGameEvent)
        .addFunction("Remove",           &GameObject::Remove)
        .addFunction("RemoveIn",         &GameObject::RemoveIn)
    );
    // clang-format on
}

GameObject::GameObject() :
    name_("Unknown"),
    id_(GetNewId()),
    stateComp_(*this)
{ }

GameObject::~GameObject()
{
    RemoveFromOctree();
}

void GameObject::UpdateRanges()
{
    ranges_.clear();
    ea::vector<GameObject*> res;

    // Compass radius
    if (QueryObjects(res, RANGE_INTEREST))
    {
        const Math::Vector3& myPos = GetPosition();
        for (const auto& o : res)
        {
            if (o->GetType() > AB::GameProtocol::GameObjectType::__SentToPlayer)
            {
                const Math::Vector3& objectPos = o->GetPosition();
                const float dist = myPos.Distance(objectPos) - AVERAGE_BB_EXTENDS;
                if (dist <= RANGE_AGGRO)
                    ranges_[Ranges::Aggro].emplace(o->id_);
                if (dist <= RANGE_COMPASS)
                    ranges_[Ranges::Compass].emplace(o->id_);
                if (dist <= RANGE_SPIRIT)
                    ranges_[Ranges::Spirit].emplace(o->id_);
                if (dist <= RANGE_EARSHOT)
                    ranges_[Ranges::Earshot].emplace(o->id_);
                if (dist <= RANGE_CASTING)
                    ranges_[Ranges::Casting].emplace(o->id_);
                if (dist <= RANGE_PROJECTILE)
                    ranges_[Ranges::Projectile].emplace(o->id_);
                if (dist <= RANGE_HALF_COMPASS)
                    ranges_[Ranges::HalfCompass].emplace(o->id_);
                if (dist <= RANGE_TOUCH)
                    ranges_[Ranges::Touch].emplace(o->id_);
                if (dist <= RANGE_ADJECENT)
                    ranges_[Ranges::Adjecent].emplace(o->id_);
                if (dist <= RANGE_VISIBLE)
                    ranges_[Ranges::Visible].emplace(o->id_);
                if (dist <= RANGE_TWO_COMPASS)
                    ranges_[Ranges::TwoCompass].emplace(o->id_);
                if (dist <= RANGE_INTEREST)
                    ranges_[Ranges::Interest].emplace(o->id_);
            }
        }
    }
}

void GameObject::Update(uint32_t timeElapsed, Net::NetworkMessage&)
{
    UpdateRanges();
    if (triggerComp_)
        triggerComp_->Update(timeElapsed);

    if (removeAt_ != 0 && removeAt_ <= Utils::Tick())
    {
        auto* disp = GetSubsystem<Asynch::Dispatcher>();
        disp->Add(Asynch::CreateTask(std::bind(&GameObject::Remove, shared_from_this())));
    }
}

bool GameObject::Collides(const GameObject* other, const Math::Vector3& velocity, Math::Vector3& move) const
{
    if (!collisionShape_ || !other || !other->GetCollisionShape())
        return false;

    bool result = false;
    GameObject* o = const_cast<GameObject*>(other);
    Collides(&o, 1, velocity, [&](GameObject&, const Math::Vector3& _move, bool&) -> Iteration {
        result = true;
        move = _move;
        return Iteration::Break;
    });
    return result;
}

void GameObject::Collides(GameObject** others, size_t count, const Math::Vector3& velocity,
    const std::function<Iteration(GameObject& other, const Math::Vector3& move, bool& updateTrans)>& callback) const
{
    auto getShape = [this]() -> ea::unique_ptr<Math::AbstractCollisionShape>
    {
        switch (collisionShape_->shapeType_)
        {
        case Math::ShapeType::BoundingBox:
            using BBoxShape = Math::CollisionShape<Math::BoundingBox>;
            return ea::make_unique<BBoxShape>(static_cast<BBoxShape&>(*collisionShape_), transformation_.GetMatrix());
        case Math::ShapeType::Sphere:
            using SphereShape = Math::CollisionShape<Math::Sphere>;
            return ea::make_unique<SphereShape>(static_cast<SphereShape&>(*collisionShape_), transformation_.GetMatrix());
        case Math::ShapeType::ConvexHull:
            using HullShape = Math::CollisionShape<Math::ConvexHull>;
            return ea::make_unique<HullShape>(static_cast<HullShape&>(*collisionShape_), transformation_.GetMatrix());
        case Math::ShapeType::HeightMap:
            using HeightShape = Math::CollisionShape<Math::HeightMap>;
            return ea::make_unique<HeightShape>(static_cast<HeightShape&>(*collisionShape_), transformation_.GetMatrix());
        default:
            ASSERT_FALSE();
        }
    };
    ea::unique_ptr<Math::AbstractCollisionShape> myTransformedShape = getShape();
    bool transformationUpdated = false;

    for (size_t i = 0; i < count; ++i)
    {
        GameObject* other = *others++;

        if (other == this || !other->collisionShape_)
            continue;

        if (transformationUpdated)
        {
            myTransformedShape = getShape();
            transformationUpdated = false;
        }

        Math::Vector3 move;
        switch (other->collisionShape_->shapeType_)
        {
        case Math::ShapeType::BoundingBox:
        {
            using BBoxShape = Math::CollisionShape<Math::BoundingBox>;
            BBoxShape* shape = static_cast<BBoxShape*>(other->GetCollisionShape());
            const Math::BoundingBox bbox = shape->Object().Transformed(other->transformation_.GetMatrix());
            if (myTransformedShape->Collides(bbox, velocity, move))
            {
#if defined(DEBUG_COLLISION)
                LOG_DEBUG << "ShapeTypeBoundingBox: this(" << *this <<
                    ") " << transformation_.position_.ToString() << " " <<
                    "collides with that(" << *other << ") " <<
                    bbox.ToString() <<
                    std::endl;
#endif
                if (callback(*other, move, transformationUpdated) != Iteration::Continue)
                    goto leave_loop;
            }
            break;
        }
        case Math::ShapeType::Sphere:
        {
            using SphereShape = Math::CollisionShape<Math::Sphere>;
            SphereShape* shape = static_cast<SphereShape*>(other->GetCollisionShape());
            const Math::Sphere sphere = shape->Object().Transformed(other->transformation_.GetMatrix());
            if (myTransformedShape->Collides(sphere, velocity, move))
            {
#if defined(DEBUG_COLLISION)
                LOG_DEBUG << "ShapeTypeSphere: this(" << *this << ") collides with that(" << *other << ")" << std::endl;
#endif
                if (callback(*other, move, transformationUpdated) != Iteration::Continue)
                    goto leave_loop;
            }
            break;
        }
        case Math::ShapeType::ConvexHull:
        {
            using HullShape = Math::CollisionShape<Math::ConvexHull>;
            HullShape* shape = static_cast<HullShape*>(other->GetCollisionShape());
            const Math::ConvexHull hull = shape->Object().Transformed(other->transformation_.GetMatrix());
            if (myTransformedShape->Collides(hull, velocity, move))
            {
#if defined(DEBUG_COLLISION)
                LOG_DEBUG << "ShapeTypeConvexHull: this(" << *this << ") collides with that(" << *other << ")" << std::endl;
#endif
                if (callback(*other, move, transformationUpdated) != Iteration::Continue)
                    goto leave_loop;
            }
            break;
        }
        case Math::ShapeType::HeightMap:
        {
            using HeightShape = Math::CollisionShape<Math::HeightMap>;
            HeightShape* shape = static_cast<HeightShape*>(other->GetCollisionShape());
            if (myTransformedShape->Collides(shape->Object(), velocity, move))
            {
#if defined(DEBUG_COLLISION)
                LOG_DEBUG << "ShapeTypeConvexHull: this(" << *this << ") collides with that(" << *other << ")" << std::endl;
#endif
                if (callback(*other, move, transformationUpdated) != Iteration::Continue)
                    goto leave_loop;
            }
            break;
        }
        default:
            ASSERT_FALSE();
        }
    }

leave_loop:;
}

const Utils::Variant& GameObject::GetVar(const std::string& name) const
{
    auto it = variables_.find(sa::StringHashRt(name.c_str()));
    if (it != variables_.end())
        return (*it).second;
    return Utils::Variant::Empty;
}

void GameObject::SetVar(const std::string& name, const Utils::Variant& val)
{
    variables_[sa::StringHashRt(name.c_str())] = val;
}

void GameObject::ProcessRayQuery(const Math::RayOctreeQuery& query, ea::vector<Math::RayQueryResult>& results)
{
    float distance = query.ray_.HitDistance(GetWorldBoundingBox());
    if (distance < query.maxDistance_)
    {
        Math::RayQueryResult result;
        result.position_ = query.ray_.origin_ + distance * query.ray_.direction_;
        result.normal_ = -query.ray_.direction_;
        result.distance_ = distance;
        result.object_ = this;
        results.push_back(std::move(result));
    }
}

bool GameObject::QueryObjects(ea::vector<GameObject*>& result, float radius)
{
    if (!octant_)
        return false;

    Math::Sphere sphere(transformation_.position_, radius);
    Math::SphereOctreeQuery query(result, sphere, this);
    Math::Octree* octree = octant_->GetRoot();
    octree->GetObjects(query);
    return true;
}

bool GameObject::QueryObjects(ea::vector<GameObject*>& result, const Math::BoundingBox& box)
{
    if (!octant_)
        return false;

    Math::BoxOctreeQuery query(result, box, this);
    Math::Octree* octree = octant_->GetRoot();
    octree->GetObjects(query);
    return true;
}

bool GameObject::Raycast(ea::vector<GameObject*>& result,
    const Math::Vector3& direction,
    float maxDist /* = Math::M_INFINITE */) const
{
    return Raycast(result, transformation_.position_ + HeadOffset, direction, maxDist);
}

bool GameObject::Raycast(ea::vector<GameObject*>& result,
    const Math::Vector3& position, const Math::Vector3& direction,
    float maxDist /* = Math::M_INFINITE */) const
{
    if (!octant_)
        return false;

    ea::vector<Math::RayQueryResult> res;
    if (!RaycastWithResult(res, position, direction, maxDist))
        return false;

    for (const auto& o : res)
        result.push_back(o.object_);
    return true;
}

bool GameObject::RaycastWithResult(ea::vector<Math::RayQueryResult>& result,
    const Math::Vector3& position, const Math::Vector3& direction,
    float maxDist /* = Math::M_INFINITE */) const
{
    if (!octant_)
        return false;

    const Math::Ray ray(position, direction);
    Math::RayOctreeQuery query(result, ray, maxDist, this);
    Math::Octree* octree = octant_->GetRoot();
    octree->Raycast(query);
    return true;
}

bool GameObject::IsObjectInSight(const GameObject& object) const
{
    ea::vector<GameObject*> result;
    const bool res = Raycast(result, object.transformation_.position_ + BodyOffset);
    if (!res)
        // Shouldn't happen
        return false;

    for (const auto* o : result)
    {
        if (!o->occluder_)
            continue;

        // result is sorted by distance
        if (o->id_ != object.id_)
        {
#ifdef DEBUG_COLLISION
            LOG_DEBUG << "Obstructed by " << *o << std::endl;
#endif
            return false;
        }
        // Can stop here it doesn't matter whats behind the target.
        return true;
    }
    return true;
}

void GameObject::Remove()
{
    if (auto game = GetGame())
        game->RemoveObject(this);
}

void GameObject::RemoveIn(uint32_t time)
{
    const int64_t newTick = Utils::Tick() + time;
    if (removeAt_ == 0 || newTick < removeAt_)
        removeAt_ = newTick;
}

void GameObject::SetState(AB::GameProtocol::CreatureState state)
{
    stateComp_.SetState(state, false);
}

std::vector<GameObject*> GameObject::_LuaQueryObjects(float radius)
{
    ea::vector<GameObject*> res;
    std::vector<GameObject*> luares;
    if (QueryObjects(res, radius))
    {
        std::copy(res.begin(), res.end(), std::back_inserter(luares));
        return luares;
    }
    return std::vector<GameObject*>();
}

void GameObject::_LuaCallGameEvent(const std::string& name, GameObject* data)
{
    if (auto game = GetGame())
        game->CallLuaEvent(name, this, data);
}

Actor* GameObject::_LuaGetClosestActor(bool undestroyable, bool unselectable)
{
    return GetClosestActor(undestroyable, unselectable);
}

std::vector<GameObject*> GameObject::_LuaGetObjectsInside()
{
    std::vector<GameObject*> result;
    if (!triggerComp_)
        return result;

    ASSERT(HasGame());
    auto game = GetGame();
    triggerComp_->VisitObjectInside([&](uint32_t id) -> Iteration
    {
        auto* object = game->GetObject<GameObject>(id);
        if (object)
            result.push_back(object);
        return Iteration::Continue;
    });
    return result;
}

bool GameObject::_LuaIsObjectInSight(const GameObject* object) const
{
    if (!object)
        return false;
    return IsObjectInSight(*object);
}

std::vector<GameObject*> GameObject::_LuaRaycast(const Math::StdVector3& direction)
{
    std::vector<GameObject*> result;

    if (!octant_)
        return result;

    const Math::Vector3& src = transformation_.position_;
    const Math::Vector3 dest(direction);
    ea::vector<Math::RayQueryResult> res;
    const Math::Ray ray(src, dest);
    Math::RayOctreeQuery query(res, ray, src.Distance(dest));
    query.ignore_ = this;
    Math::Octree* octree = octant_->GetRoot();
    octree->Raycast(query);
    for (const auto& o : query.result_)
    {
        result.push_back(o.object_);
    }
    return result;
}

std::vector<GameObject*> GameObject::_LuaRaycastTo(const Math::StdVector3& destination)
{
    const Math::Vector3& src = transformation_.position_;
    const Math::Vector3 direction = Math::Vector3(destination) - src;
    return _LuaRaycast(direction);
}

bool GameObject::IsInRange(Ranges range, const GameObject* object) const
{
    if (!object)
        return false;
    if (range == Ranges::Map)
        return true;
    // Don't calculate the distance now, but use previously calculated values.
    const auto rIt = ranges_.find(range);
    if (rIt == ranges_.end())
        return false;
    const auto& r = (*rIt).second;
    const auto it = r.find(object->id_);
    return it != r.end();
}

bool GameObject::IsCloserThan(float maxDist, const GameObject* object) const
{
    return GetDistance(object) < maxDist;
}

std::vector<Actor*> GameObject::_LuaGetActorsInRange(Ranges range) const
{
    std::vector<Actor*> result;
    VisitInRange(range, [&](GameObject& o)
    {
        if (o.IsPlayerOrNpcType())
            result.push_back(To<Actor>(&o));
        return Iteration::Continue;
    });
    return result;
}

Actor* GameObject::GetClosestActor(const std::function<bool(const Actor& actor)>& callback)
{
    Actor* result = nullptr;
    for (unsigned r = static_cast<unsigned>(Ranges::Aggro); r < static_cast<unsigned>(Ranges::Map); ++r)
    {
        VisitInRange(static_cast<Ranges>(r), [&](GameObject& o)
        {
            if (o.IsPlayerOrNpcType())
            {
                result = To<Actor>(&o);
                if (!callback(*result))
                {
                    result = nullptr;
                    return Iteration::Continue;
                }
                return Iteration::Break;
            }
            return Iteration::Continue;
        });
        if (result)
            break;
    }
    return result;
}

Actor* GameObject::GetClosestActor(bool undestroyable, bool unselectable)
{
    return GetClosestActor([&](const Actor& actor) -> bool
    {
        if (!actor.IsSelectable() && !unselectable)
            return false;
        if ((actor.IsUndestroyable() || actor.IsDead()) && !undestroyable)
            return false;
        return true;
    });
}

Actor* GameObject::_LuaAsActor()
{
    return To<Actor>(this);
}

Npc* GameObject::_LuaAsNpc()
{
    return To<Npc>(this);
}

Player* GameObject::_LuaAsPlayer()
{
    return To<Player>(this);
}

AreaOfEffect* GameObject::_LuaAsAOE()
{
    return To<AreaOfEffect>(this);
}

void GameObject::_LuaSetPosition(const Math::StdVector3& pos)
{
    transformation_.position_ = pos;
}

void GameObject::_LuaSetRotation(float y)
{
    float ang = Math::DegToRad(y);
    Math::NormalizeAngle(ang);
    transformation_.SetYRotation(ang);
}

void GameObject::_LuaSetScale(const Math::StdVector3& scale)
{
    transformation_.scale_ = scale;
}

void GameObject::_LuaSetScaleSimple(float value)
{
    transformation_.scale_.x_ = value;
    transformation_.scale_.y_ = value;
    transformation_.scale_.z_ = value;
}

Math::StdVector3 GameObject::_LuaGetPosition() const
{
    return static_cast<Math::StdVector3>(transformation_.position_);
}

float GameObject::_LuaGetRotation() const
{
    return Math::RadToDeg(transformation_.GetYRotation());
}

Math::StdVector3 GameObject::_LuaGetScale() const
{
    return static_cast<Math::StdVector3>(transformation_.scale_);
}

void GameObject::_LuaSetBoundingBox(const Math::StdVector3& min, const Math::StdVector3& max)
{
    if (!collisionShape_)
        return;
    if (collisionShape_->shapeType_ == Math::ShapeType::BoundingBox)
    {
        using BBoxShape = Math::CollisionShape<Math::BoundingBox>;
        BBoxShape* shape = static_cast<BBoxShape*>(GetCollisionShape());
        auto& obj = shape->Object();
        obj.min_ = min;
        obj.max_ = max;
    }
}

void GameObject::SetBoundingSize(const Math::Vector3& size)
{
    if (!collisionShape_)
        return;
    switch (collisionShape_->shapeType_)
    {
    case Math::ShapeType::BoundingBox:
    {
        const Math::Vector3 halfSize = (size * 0.5f);
        using BBoxShape = Math::CollisionShape<Math::BoundingBox>;
        BBoxShape* shape = static_cast<BBoxShape*>(GetCollisionShape());
        auto& obj = shape->Object();
        obj.min_ = -halfSize;
        obj.max_ = halfSize;
        break;
    }
    case Math::ShapeType::Sphere:
    {
        using SphereShape = Math::CollisionShape<Math::Sphere>;
        SphereShape* shape = static_cast<SphereShape*>(GetCollisionShape());
        auto& obj = shape->Object();
        obj.radius_ = size.x_ * 0.5f;
        break;
    }
    default:
        // Not possible for other shapes
        LOG_WARNING << "Can not set size of shape type " << static_cast<int>(collisionShape_->shapeType_) << std::endl;
        break;
    }
}

void GameObject::_LuaSetBoundingSize(const Math::StdVector3& size)
{
    SetBoundingSize(size);
}

std::string GameObject::_LuaGetVarString(const std::string& name)
{
    return GetVar(name).GetString();
}

void GameObject::_LuaSetVarString(const std::string& name, const std::string& value)
{
    SetVar(name, Utils::Variant(value));
}

float GameObject::_LuaGetVarNumber(const std::string& name)
{
    return GetVar(name).GetFloat();
}

void GameObject::_LuaSetVarNumber(const std::string& name, float value)
{
    SetVar(name, Utils::Variant(value));
}

Game* GameObject::_LuaGetGame()
{
    if (auto g = game_.lock())
        return g.get();
    return nullptr;
}

int GameObject::_LuaGetState()
{
    return static_cast<int>(stateComp_.GetState());
}

void GameObject::_LuaSetState(int state)
{
    SetState(static_cast<AB::GameProtocol::CreatureState>(state));
}

void GameObject::AddToOctree()
{
    if (auto g = game_.lock())
    {
#ifdef DEBUG_OCTREE
        LOG_DEBUG << "Adding " << *this << " to Octree" << std::endl;
#endif
        g->map_->octree_->InsertObject(this);
        // Initial update.
        if (octant_)
        {
            Math::Octree* octree = octant_->GetRoot();
            octree->AddObjectUpdate(this);
        }
#ifdef DEBUG_OCTREE
        else
        {
            LOG_WARNING << "octant_ == null" << std::endl;
        }
#endif
    }
}

void GameObject::RemoveFromOctree()
{
    if (octant_)
    {
#ifdef DEBUG_OCTREE
        LOG_DEBUG << "Removing " << *this << " from Octree" << std::endl;
#endif
        octant_->RemoveObject(this);
    }
}

void GameObject::WriteSpawnData(Net::NetworkMessage& msg)
{
    msg.Add<uint32_t>(id_);
    msg.Add<uint8_t>(static_cast<uint8_t>(GetType()));
}

bool GameObject::Serialize(sa::PropWriteStream& stream)
{
    stream.WriteString(GetName());
    return true;
}

bool GameObject::IsInOutpost() const
{
    if (auto g = game_.lock())
        return AB::Entities::IsOutpost(g->data_.type);
    return false;
}

uint32_t GameObject::GetRetriggerTimout() const
{
    if (!triggerComp_)
        return 0;
    return triggerComp_->retriggerTimeout_;
}

void GameObject::SetRetriggerTimout(uint32_t value)
{
    if (!triggerComp_)
        triggerComp_ = ea::make_unique<Components::TriggerComp>(*this);
    triggerComp_->retriggerTimeout_ = value;
}

bool GameObject::IsTrigger() const
{
    return triggerComp_ && triggerComp_->trigger_;
}

void GameObject::SetTrigger(bool value)
{
    if (!triggerComp_)
        triggerComp_ = ea::make_unique<Components::TriggerComp>(*this);
    triggerComp_->trigger_ = value;
}

void GameObject::VisitInRange(Ranges range, const std::function<Iteration(GameObject&)>& func) const
{
    auto game = GetGame();
    ASSERT(game);
    const auto r = ranges_.find(range);
    if (r == ranges_.end())
        return;
    for (uint32_t objectId : (*r).second)
    {
        auto* object = game->GetObject<GameObject>(objectId);
        if (object)
        {
            if (func(*object) != Iteration::Continue)
                break;
        }
    }
}

}
