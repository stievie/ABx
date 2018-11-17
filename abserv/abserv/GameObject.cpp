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

#include "DebugNew.h"

namespace Game {

Utils::IdGenerator<uint32_t> GameObject::objectIds_;
// Let's make the head 1.6m above the ground
const Math::Vector3 Actor::HeadOffset(0.0f, 1.8f, 0.0f);
const Math::Vector3 Actor::BodyOffset(0.0f, 1.0f, 0.0f);

void GameObject::RegisterLua(kaguya::State& state)
{
    state["GameObject"].setClass(kaguya::UserdataMetatable<GameObject>()
        .addFunction("GetId", &GameObject::GetId)
        .addFunction("GetGame", &GameObject::GetGame)
        .addFunction("GetName", &GameObject::GetName)
        .addFunction("GetCollisionMask", &GameObject::GetCollisionMask)
        .addFunction("SetCollisionMask", &GameObject::SetCollisionMask)
        .addFunction("QueryObjects", &GameObject::_LuaQueryObjects)
        .addFunction("Raycast", &GameObject::_LuaRaycast)
        .addFunction("SetBoundingBox", &GameObject::_LuaSetBoundingBox)
        .addFunction("GetVarString", &GameObject::_LuaGetVarString)
        .addFunction("SetVarString", &GameObject::_LuaSetVarString)
        .addFunction("GetVarNumber", &GameObject::_LuaGetVarNumber)
        .addFunction("SetVarNumber", &GameObject::_LuaSetVarNumber)

        .addFunction("SetPosition", &GameObject::_LuaSetPosition)
        .addFunction("SetRotation", &GameObject::_LuaSetRotation)
        .addFunction("SetScale", &GameObject::_LuaSetScale)
        .addFunction("GetPosition", &GameObject::_LuaGetPosition)
        .addFunction("GetRotation", &GameObject::_LuaGetRotation)
        .addFunction("GetScale", &GameObject::_LuaGetScale)

        // Can return empty if up-cast is not possible
        .addFunction("AsActor", &GameObject::_LuaAsActor)
        .addFunction("AsNpc", &GameObject::_LuaAsNpc)
        .addFunction("AsPlayer", &GameObject::_LuaAsPlayer)
    );
}

GameObject::GameObject() :
    stateComp_(*this),
    octant_(nullptr),
    sortValue_(0.0f),
    occludee_(true),
    occluder_(false),
    name_("Unknown"),
    id_(GetNewId()),
    collisionMask_(0xFFFFFFFF)    // Collides with all by default
{
}

GameObject::~GameObject()
{
    RemoveFromOctree();
}

void GameObject::Update(uint32_t, Net::NetworkMessage&)
{
}

bool GameObject::Collides(GameObject* other, Math::Vector3& move) const
{
    if (!collisionShape_ || !other->GetCollisionShape())
        return false;

    switch (other->GetCollisionShape()->shapeType_)
    {
    case Math::ShapeTypeBoundingBox:
    {
        using BBoxShape = Math::CollisionShapeImpl<Math::BoundingBox>;
        BBoxShape* shape = (BBoxShape*)other->GetCollisionShape();
        const Math::BoundingBox bbox = shape->shape_->Transformed(other->transformation_.GetMatrix());
#if defined(DEBUG_COLLISION)
        bool ret = false;
        ret = collisionShape_->Collides(transformation_.GetMatrix(), bbox, move);
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
        return collisionShape_->Collides(transformation_.GetMatrix(), bbox, move);
#endif
    }
    case Math::ShapeTypeSphere:
    {
        using SphereShape = Math::CollisionShapeImpl<Math::Sphere>;
        SphereShape* shape = (SphereShape*)other->GetCollisionShape();
        const Math::Sphere sphere = shape->shape_->Transformed(other->transformation_.GetMatrix());
#if defined(DEBUG_COLLISION)
        bool ret = false;
        ret = collisionShape_->Collides(transformation_.GetMatrix(), sphere, move);
        if (ret)
        {
            LOG_INFO << "ShapeTypeSphere: this(" << GetName() << ") collides with that(" << other->GetName() << ")" << std::endl;
        }
        return ret;
#else
        return collisionShape_->Collides(transformation_.GetMatrix(), sphere, move);
#endif
    }
    case Math::ShapeTypeConvexHull:
    {
        using HullShape = Math::CollisionShapeImpl<Math::ConvexHull>;
        HullShape* shape = (HullShape*)other->GetCollisionShape();
        const Math::ConvexHull hull = shape->shape_->Transformed(other->transformation_.GetMatrix());
#if defined(DEBUG_COLLISION)
        bool ret = false;
        ret = collisionShape_->Collides(transformation_.GetMatrix(), hull, move);
        if (ret)
        {
            LOG_INFO << "ShapeTypeConvexHull: this(" << GetName() << ") collides with that(" << other->GetName() << ")" << std::endl;
        }
        return ret;
#else
        return collisionShape_->Collides(transformation_.GetMatrix(), hull, move);
#endif
    }
    case Math::ShapeTypeHeightMap:
    {
        using HeightShape = Math::CollisionShapeImpl<Math::HeightMap>;
        HeightShape* shape = (HeightShape*)other->GetCollisionShape();
#if defined(DEBUG_COLLISION)
        bool ret = false;
        ret = collisionShape_->Collides(transformation_.GetMatrix(), *shape->shape_, move);
        if (ret)
        {
            LOG_INFO << "ShapeTypeConvexHull: this(" << GetName() << ") collides with that(" << other->GetName() << ")" << std::endl;
        }
        return ret;
#else
        return collisionShape_->Collides(transformation_.GetMatrix(), *shape->shape_, move);
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

void GameObject::OnCollide(std::shared_ptr<Actor> actor)
{
}

bool GameObject::Raycast(std::vector<GameObject*>& result, const Math::Vector3& direction)
{
    if (!octant_)
        return false;

    std::vector<Math::RayQueryResult> res;
    Math::Ray ray(transformation_.position_ + HeadOffset, direction);
    Math::RayOctreeQuery query(res, ray);
    Math::Octree* octree = octant_->GetRoot();
    octree->Raycast(query);
    for (const auto& o : query.result_)
    {
        result.push_back(o.object_);
    }
    return true;
}

std::vector<std::shared_ptr<GameObject>> GameObject::_LuaQueryObjects(float radius)
{
    std::vector<GameObject*> res;
    if (QueryObjects(res, radius))
    {
        std::vector<std::shared_ptr<GameObject>> result;
        for (const auto& o : res)
        {
            if (o != this)
                result.push_back(o->GetThis<GameObject>());
        }
        return result;
    }
    return std::vector<std::shared_ptr<GameObject>>();
}

std::vector<std::shared_ptr<GameObject>> GameObject::_LuaRaycast(float x, float y, float z)
{
    std::vector<std::shared_ptr<GameObject>> result;

    if (!octant_)
        return result;

    const Math::Vector3& src = transformation_.position_;
    const Math::Vector3 dest(x, y, z);
    std::vector<Math::RayQueryResult> res;
    Math::Ray ray(src, dest);
    Math::RayOctreeQuery query(res, ray, src.Distance(dest));
    Math::Octree* octree = octant_->GetRoot();
    octree->Raycast(query);
    for (const auto& o : query.result_)
    {
        if (o.object_ != this)
            result.push_back(o.object_->GetThis<GameObject>());
    }
    return result;
}

std::shared_ptr<Actor> GameObject::_LuaAsActor()
{
    return std::dynamic_pointer_cast<Actor>(shared_from_this());
}

std::shared_ptr<Npc> GameObject::_LuaAsNpc()
{
    if (GetType() == AB::GameProtocol::ObjectTypeNpc)
        return std::dynamic_pointer_cast<Npc>(shared_from_this());
    return std::shared_ptr<Npc>();
}

std::shared_ptr<Player> GameObject::_LuaAsPlayer()
{
    if (GetType() == AB::GameProtocol::ObjectTypePlayer)
        return std::dynamic_pointer_cast<Player>(shared_from_this());
    return std::shared_ptr<Player>();
}

void GameObject::_LuaSetPosition(float x, float y, float z)
{
    transformation_.position_.x_ = x;
    transformation_.position_.y_ = y;
    transformation_.position_.z_ = z;
}

void GameObject::_LuaSetRotation(float y)
{
    transformation_.rotation_ = Math::DegToRad(y);
}

void GameObject::_LuaSetScale(float x, float y, float z)
{
    transformation_.scale_.x_ = x;
    transformation_.scale_.y_ = y;
    transformation_.scale_.z_ = z;
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
    return transformation_.rotation_;
}

std::vector<float> GameObject::_LuaGetScale() const
{
    std::vector<float> result;
    result.push_back(transformation_.scale_.x_);
    result.push_back(transformation_.scale_.y_);
    result.push_back(transformation_.scale_.z_);
    return result;
}

void GameObject::_LuaSetBoundingBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
{
    if (collisionShape_ && collisionShape_->shapeType_ == Math::ShapeTypeBoundingBox)
    {
        using BBoxShape = Math::CollisionShapeImpl<Math::BoundingBox>;
        BBoxShape* shape = (BBoxShape*)GetCollisionShape();
        shape->shape_->min_ = Math::Vector3(minX, minY, minZ);
        shape->shape_->max_ = Math::Vector3(maxX, maxY, maxZ);
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
