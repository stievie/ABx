#include "stdafx.h"
#include "GameObject.h"
#include "Game.h"
#include "Logger.h"
#include "ConvexHull.h"
#include "Actor.h"
#include "Npc.h"
#include "Player.h"
#include "MathUtils.h"
#include "ConfigManager.h"
#include "Scheduler.h"

#include "DebugNew.h"

namespace Game {

Utils::IdGenerator<uint32_t> GameObject::objectIds_;
// Let's make the head 1.6m above the ground
const Math::Vector3 GameObject::HeadOffset(0.0f, 1.7f, 0.0f);
const Math::Vector3 GameObject::BodyOffset(0.0f, 1.0f, 0.0f);

void GameObject::RegisterLua(kaguya::State& state)
{
    state["GameObject"].setClass(kaguya::UserdataMetatable<GameObject>()
        .addFunction("GetId",            &GameObject::GetId)
        .addFunction("GetGame",          &GameObject::_LuaGetGame)
        .addFunction("GetName",          &GameObject::GetName)
        .addFunction("GetCollisionMask", &GameObject::GetCollisionMask)
        .addFunction("SetCollisionMask", &GameObject::SetCollisionMask)
        .addFunction("QueryObjects",     &GameObject::_LuaQueryObjects)
        .addFunction("Raycast",          &GameObject::_LuaRaycast)
        .addFunction("SetBoundingBox",   &GameObject::_LuaSetBoundingBox)
        .addFunction("SetBoundingSize",  &GameObject::_LuaSetBoundingSize)
        .addFunction("GetVarString",     &GameObject::_LuaGetVarString)
        .addFunction("SetVarString",     &GameObject::_LuaSetVarString)
        .addFunction("GetVarNumber",     &GameObject::_LuaGetVarNumber)
        .addFunction("SetVarNumber",     &GameObject::_LuaSetVarNumber)
        .addFunction("IsTrigger",        &GameObject::IsTrigger)
        .addFunction("SetTrigger",       &GameObject::SetTrigger)

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

        .addFunction("GetActorsInRange", &GameObject::GetActorsInRange)
        .addFunction("IsInRange",        &GameObject::IsInRange)
        .addFunction("IsObjectInSight",  &GameObject::IsObjectInSight)
        .addFunction("CallGameEvent",    &GameObject::_LuaCallGameEvent)
        .addFunction("Remove",           &GameObject::Remove)
        .addFunction("RemoveIn",         &GameObject::RemoveIn)
    );
}

GameObject::GameObject() :
    collisionShape_(nullptr),
    octant_(nullptr),
    sortValue_(0.0f),
    id_(GetNewId()),
    name_("Unknown"),
    stateComp_(*this),
    triggerComp_(nullptr),         // By default its not a trigger
    occluder_(false),
    occludee_(true),
    collisionMask_(0xFFFFFFFF)    // Collides with all by default
{
}

GameObject::~GameObject()
{
    RemoveFromOctree();
}

void GameObject::UpdateRanges()
{
    ranges_.clear();
    std::vector<GameObject*> res;

    // Compass radius
    if (QueryObjects(res, RANGE_COMPASS))
    {
        for (const auto& o : res)
        {
            if (o != this && o->GetType() > AB::GameProtocol::ObjectTypeSentToPlayer)
            {
                auto so = o->shared_from_this();
                const Math::Vector3 objectPos = o->GetPosition();
                const Math::Vector3 myPos = GetPosition();
                const float dist = myPos.Distance(objectPos);
                if (dist <= RANGE_AGGRO)
                    ranges_[Ranges::Aggro].push_back(so);
                if (dist <= RANGE_COMPASS)
                    ranges_[Ranges::Compass].push_back(so);
                if (dist <= RANGE_SPIRIT)
                    ranges_[Ranges::Spirit].push_back(so);
                if (dist <= RANGE_EARSHOT)
                    ranges_[Ranges::Earshot].push_back(so);
                if (dist <= RANGE_CASTING)
                    ranges_[Ranges::Casting].push_back(so);
                if (dist <= RANGE_PROJECTILE)
                    ranges_[Ranges::Projectile].push_back(so);
                if (dist <= RANGE_HALF_COMPASS)
                    ranges_[Ranges::HalfCompass].push_back(so);
                if (dist <= RANGE_TOUCH)
                    ranges_[Ranges::Touch].push_back(so);
                if (dist <= RANGE_ADJECENT)
                    ranges_[Ranges::Adjecent].push_back(so);
                if (dist <= RANGE_VISIBLE)
                    ranges_[Ranges::Visible].push_back(so);
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

bool GameObject::Collides(GameObject* other, const Math::Vector3& velocity, Math::Vector3& move) const
{
    if (!collisionShape_ || !other || !other->GetCollisionShape())
        return false;

    switch (other->GetCollisionShape()->shapeType_)
    {
    case Math::ShapeType::BoundingBox:
    {
        using BBoxShape = Math::CollisionShapeImpl<Math::BoundingBox>;
        BBoxShape* shape = static_cast<BBoxShape*>(other->GetCollisionShape());
        const Math::BoundingBox bbox = shape->Object()->Transformed(other->transformation_.GetMatrix());
#if defined(DEBUG_COLLISION)
        bool ret = false;
        ret = collisionShape_->Collides(transformation_.GetMatrix(), bbox, velocity, move);
        if (ret)
        {
            LOG_DEBUG << "ShapeTypeBoundingBox: this(" << GetName() <<
                ") " << transformation_.position_.ToString() << " " <<
                "collides with that(" << other->GetName() << ") " <<
                bbox.ToString() <<
                std::endl;
        }
        return ret;
#else
        return collisionShape_->Collides(transformation_.GetMatrix(), bbox, velocity, move);
#endif
    }
    case Math::ShapeType::Sphere:
    {
        using SphereShape = Math::CollisionShapeImpl<Math::Sphere>;
        SphereShape* shape = static_cast<SphereShape*>(other->GetCollisionShape());
        const Math::Sphere sphere = shape->Object()->Transformed(other->transformation_.GetMatrix());
#if defined(DEBUG_COLLISION)
        bool ret = false;
        ret = collisionShape_->Collides(transformation_.GetMatrix(), sphere, velocity, move);
        if (ret)
        {
            LOG_INFO << "ShapeTypeSphere: this(" << GetName() << ") collides with that(" << other->GetName() << ")" << std::endl;
        }
        return ret;
#else
        return collisionShape_->Collides(transformation_.GetMatrix(), sphere, velocity,  move);
#endif
    }
    case Math::ShapeType::ConvexHull:
    {
        using HullShape = Math::CollisionShapeImpl<Math::ConvexHull>;
        HullShape* shape = static_cast<HullShape*>(other->GetCollisionShape());
        const Math::ConvexHull hull = shape->Object()->Transformed(other->transformation_.GetMatrix());
#if defined(DEBUG_COLLISION)
        bool ret = false;
        ret = collisionShape_->Collides(transformation_.GetMatrix(), hull, velocity, move);
        if (ret)
        {
            LOG_INFO << "ShapeTypeConvexHull: this(" << GetName() << ") collides with that(" << other->GetName() << ")" << std::endl;
        }
        return ret;
#else
        return collisionShape_->Collides(transformation_.GetMatrix(), hull, velocity, move);
#endif
    }
    case Math::ShapeType::HeightMap:
    {
        using HeightShape = Math::CollisionShapeImpl<Math::HeightMap>;
        HeightShape* shape = static_cast<HeightShape*>(other->GetCollisionShape());
#if defined(DEBUG_COLLISION)
        bool ret = false;
        ret = collisionShape_->Collides(transformation_.GetMatrix(), *shape->Object(), velocity, move);
        if (ret)
        {
            LOG_INFO << "ShapeTypeConvexHull: this(" << GetName() << ") collides with that(" << other->GetName() << ")" << std::endl;
        }
        return ret;
#else
        return collisionShape_->Collides(transformation_.GetMatrix(), *shape->Object(), velocity, move);
#endif
    }
    }
    return false;
}

const Utils::Variant& GameObject::GetVar(const std::string& name) const
{
    auto it = variables_.find(Utils::StringHashRt(name.c_str()));
    if (it != variables_.end())
        return (*it).second;
    return Utils::Variant::Empty;
}

void GameObject::SetVar(const std::string& name, const Utils::Variant& val)
{
    variables_[Utils::StringHashRt(name.c_str())] = val;
}

void GameObject::ProcessRayQuery(const Math::RayOctreeQuery& query, std::vector<Math::RayQueryResult>& results)
{
    float distance = query.ray_.HitDistance(GetWorldBoundingBox());
    if (distance < query.maxDistance_)
    {
        Math::RayQueryResult result;
        result.position_ = query.ray_.origin_ + distance * query.ray_.direction_;
        result.normal_ = -query.ray_.direction_;
        result.distance_ = distance;
        result.object_ = this;
        results.push_back(result);
    }
}

bool GameObject::QueryObjects(std::vector<GameObject*>& result, float radius)
{
    if (!octant_)
        return false;

    Math::Sphere sphere(transformation_.position_, radius);
    Math::SphereOctreeQuery query(result, sphere);
    Math::Octree* octree = octant_->GetRoot();
    octree->GetObjects(query);
    return true;
}

bool GameObject::QueryObjects(std::vector<GameObject*>& result, const Math::BoundingBox& box)
{
    if (!octant_)
        return false;

    Math::BoxOctreeQuery query(result, box);
    Math::Octree* octree = octant_->GetRoot();
    octree->GetObjects(query);
    return true;
}

void GameObject::OnCollide(GameObject* other)
{
    if (triggerComp_)
        triggerComp_->OnCollide(other);
}

bool GameObject::Raycast(std::vector<GameObject*>& result, const Math::Vector3& direction) const
{
    return Raycast(result, transformation_.position_ + HeadOffset, direction);
}

bool GameObject::Raycast(std::vector<GameObject*>& result, const Math::Vector3& position, const Math::Vector3& direction) const
{
    if (!octant_)
        return false;

    std::vector<Math::RayQueryResult> res;
    Math::Ray ray(position, direction);
    Math::RayOctreeQuery query(res, ray);
    Math::Octree* octree = octant_->GetRoot();
    octree->Raycast(query);
    for (const auto& o : query.result_)
        result.push_back(o.object_);
    return true;
}

bool GameObject::IsObjectInSight(const GameObject* object) const
{
    assert(object);
    std::vector<GameObject*> result;
    const bool res = Raycast(result, object->transformation_.position_);
    if (res)
    {
        for (const auto* o : result)
        {
            // result is sorted by distance
            if (o->id_ != id_ && o->id_ != object->id_)
                return false;
            else if (o->id_ == object->id_)
                // Can stop here it doesn't matter whats behind the target.
                return true;
        }
        // Actually shouldn't get here
        return true;
    }
    // Shouldn't happen
    return false;
}

void GameObject::Remove()
{
    auto game = GetGame();
    if (game)
        game->RemoveObject(this);
}

void GameObject::RemoveIn(uint32_t time)
{
    const int64_t newTick = Utils::Tick() + time;
    if (removeAt_ == 0 || newTick < removeAt_)
        removeAt_ = newTick;
}

std::vector<GameObject*> GameObject::_LuaQueryObjects(float radius)
{
    std::vector<GameObject*> res;
    if (QueryObjects(res, radius))
    {
        std::vector<GameObject*> result;
        for (const auto& o : res)
        {
            if (o != this)
                result.push_back(o);
        }
        return result;
    }
    return std::vector<GameObject*>();
}

void GameObject::_LuaCallGameEvent(const std::string& name, GameObject* data)
{
    auto game = GetGame();
    if (game)
        game->CallLuaEvent(name, this, data);
}

std::vector<GameObject*> GameObject::_LuaRaycast(float x, float y, float z)
{
    std::vector<GameObject*> result;

    if (!octant_)
        return result;

    const Math::Vector3& src = transformation_.position_;
    const Math::Vector3 dest(x, y, z);
    std::vector<Math::RayQueryResult> res;
    const Math::Ray ray(src, dest);
    Math::RayOctreeQuery query(res, ray, src.Distance(dest));
    Math::Octree* octree = octant_->GetRoot();
    octree->Raycast(query);
    for (const auto& o : query.result_)
    {
        if (o.object_ != this)
            result.push_back(o.object_);
    }
    return result;
}

std::vector<Actor*> GameObject::GetActorsInRange(Ranges range)
{
    std::vector<Actor*> result;
    VisitInRange(range, [&](GameObject& o)
    {
        const AB::GameProtocol::GameObjectType t = o.GetType();
        if (t == AB::GameProtocol::ObjectTypeNpc || t == AB::GameProtocol::ObjectTypePlayer)
            result.push_back(dynamic_cast<Actor*>(&o));
        return Iteration::Continue;
    });
    return result;
}

Actor* GameObject::_LuaAsActor()
{
    return dynamic_cast<Actor*>(this);
}

Npc* GameObject::_LuaAsNpc()
{
    if (GetType() == AB::GameProtocol::ObjectTypeNpc)
        return dynamic_cast<Npc*>(this);
    return nullptr;
}

Player* GameObject::_LuaAsPlayer()
{
    if (GetType() == AB::GameProtocol::ObjectTypePlayer)
        return dynamic_cast<Player*>(this);
    return nullptr;
}

void GameObject::_LuaSetPosition(float x, float y, float z)
{
    transformation_.position_.x_ = x;
    transformation_.position_.y_ = y;
    transformation_.position_.z_ = z;
}

void GameObject::_LuaSetRotation(float y)
{
    float ang = Math::DegToRad(y);
    Math::NormalizeAngle(ang);
    transformation_.SetYRotation(ang);
}

void GameObject::_LuaSetScale(float x, float y, float z)
{
    transformation_.scale_.x_ = x;
    transformation_.scale_.y_ = y;
    transformation_.scale_.z_ = z;
}

void GameObject::_LuaSetScaleSimple(float value)
{
    transformation_.scale_.x_ = value;
    transformation_.scale_.y_ = value;
    transformation_.scale_.z_ = value;
}

std::vector<float> GameObject::_LuaGetPosition() const
{
    std::vector<float> result;
    result.push_back(transformation_.position_.x_);
    result.push_back(transformation_.position_.y_);
    result.push_back(transformation_.position_.z_);
    return result;
}

float GameObject::_LuaGetRotation() const
{
    return Math::RadToDeg(transformation_.GetYRotation());
}

std::vector<float> GameObject::_LuaGetScale() const
{
    std::vector<float> result;
    result.push_back(transformation_.scale_.x_);
    result.push_back(transformation_.scale_.y_);
    result.push_back(transformation_.scale_.z_);
    return result;
}

void GameObject::_LuaSetBoundingBox(
    float minX, float minY, float minZ,
    float maxX, float maxY, float maxZ)
{
    if (!collisionShape_)
        return;
    if (collisionShape_->shapeType_ == Math::ShapeType::BoundingBox)
    {
        using BBoxShape = Math::CollisionShapeImpl<Math::BoundingBox>;
        BBoxShape* shape = static_cast<BBoxShape*>(GetCollisionShape());
        auto* obj = shape->Object();
        obj->min_ = Math::Vector3(minX, minY, minZ);
        obj->max_ = Math::Vector3(maxX, maxY, maxZ);
    }
}

void GameObject::_LuaSetBoundingSize(float x, float y, float z)
{
    if (!collisionShape_)
        return;
    switch (collisionShape_->shapeType_)
    {
    case Math::ShapeType::BoundingBox:
    {
        const Math::Vector3 halfSize = (Math::Vector3(x, y, z) * 0.5f);
        using BBoxShape = Math::CollisionShapeImpl<Math::BoundingBox>;
        BBoxShape* shape = static_cast<BBoxShape*>(GetCollisionShape());
        auto* obj = shape->Object();
        obj->min_ = -halfSize;
        obj->max_ = halfSize;
        break;
    }
    case Math::ShapeType::Sphere:
    {
        using SphereShape = Math::CollisionShapeImpl<Math::Sphere>;
        SphereShape* shape = static_cast<SphereShape*>(GetCollisionShape());
        auto* obj = shape->Object();
        obj->radius_ = x * 0.5f;
        break;
    }
    default:
        // Not possible for other shapes
        LOG_WARNING << "Can not set size of shape type " << static_cast<int>(collisionShape_->shapeType_) << std::endl;
        break;
    }
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

void GameObject::AddToOctree()
{
    if (auto g = game_.lock())
    {
#ifdef DEBUG_OCTREE
        LOG_DEBUG << "Adding " << GetName() << " to Octree" << std::endl;
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
        LOG_DEBUG << "Removing " << GetName() << " from Octree" << std::endl;
#endif
        octant_->RemoveObject(this);
    }
}

bool GameObject::Serialize(IO::PropWriteStream& stream)
{
    stream.Write<uint8_t>(GetType());
    stream.WriteString(GetName());
    return true;
}

}
