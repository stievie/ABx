#include "stdafx.h"
#include "GameObject.h"
#include "Game.h"
#include "Logger.h"
#include "ConvexHull.h"
#include "Creature.h"
#include "Npc.h"
#include "Player.h"

#include "DebugNew.h"

namespace Game {

uint32_t GameObject::objectIds_ = 0;

void GameObject::RegisterLua(kaguya::State& state)
{
    state["GameObject"].setClass(kaguya::UserdataMetatable<GameObject>()
        .addFunction("GetId", &GameObject::GetId)
        .addFunction("GetGame", &GameObject::GetGame)
        .addFunction("GetName", &GameObject::GetName)
        .addFunction("GetCollisionMask", &GameObject::GetCollisionMask)
        .addFunction("SetCollisionMask", &GameObject::SetCollisionMask)
        .addFunction("QueryObjects", &GameObject::_LuaQueryObjects)
        // Can return empty if up-cast is not possible
        .addFunction("AsCreature", &GameObject::_LuaAsCreature)
        .addFunction("AsNpc", &GameObject::_LuaAsNpc)
        .addFunction("AsPlayer", &GameObject::_LuaAsPlayer)
    );
}

GameObject::GameObject() :
    octant_(nullptr),
    sortValue_(0.0f),
    occludee_(true),
    occluder_(false),
    name_("Unknown"),
    creatureState_(AB::GameProtocol::CreatureStateIdle),
    collisionMask_(0xFFFFFFFF)    // Collides with all by default
{
    id_ = GetNewId();
}

GameObject::~GameObject()
{
    RemoveFromOctree();
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
        const Math::BoundingBox bbox = shape->shape_.Transformed(other->transformation_.GetMatrix());
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
        const Math::Sphere sphere = shape->shape_.Transformed(other->transformation_.GetMatrix());
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
        const Math::ConvexHull hull = shape->shape_.Transformed(other->transformation_.GetMatrix());
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
        ret = collisionShape_->Collides(transformation_.GetMatrix(), shape->shape_, move);
        if (ret)
        {
            LOG_INFO << "ShapeTypeConvexHull: this(" << GetName() << ") collides with that(" << other->GetName() << ")" << std::endl;
        }
        return ret;
#else
        return collisionShape_->Collides(transformation_.GetMatrix(), shape->shape_, move);
#endif
    }
    }
    return false;
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

bool GameObject::Serialize(IO::PropWriteStream& stream)
{
    stream.Write<uint8_t>(GetType());
    stream.WriteString(GetName());
    return true;
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

std::shared_ptr<Creature> GameObject::_LuaAsCreature()
{
    return std::dynamic_pointer_cast<Creature>(shared_from_this());
}

std::shared_ptr<Npc> GameObject::_LuaAsNpc()
{
    return std::dynamic_pointer_cast<Npc>(shared_from_this());
}

std::shared_ptr<Player> GameObject::_LuaAsPlayer()
{
    return std::dynamic_pointer_cast<Player>(shared_from_this());
}

void GameObject::AddToOctree()
{
    if (auto g = game_.lock())
    {
#ifdef DEBUG_GAME
        LOG_DEBUG << "Adding " << GetName() << " to Octree" << std::endl;
#endif
        g->map_->octree_->InsertObject(this);
        // Initial update.
        if (octant_)
        {
            Math::Octree* octree = octant_->GetRoot();
            octree->AddObjectUpdate(this);
        }
    }
}

void GameObject::RemoveFromOctree()
{
    if (octant_)
    {
#ifdef DEBUG_GAME
        LOG_DEBUG << "Removing " << GetName() << " from Octree" << std::endl;
#endif
        octant_->RemoveObject(this);
    }
}

}
