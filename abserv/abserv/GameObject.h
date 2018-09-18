#pragma once

#include "Vector3.h"
#include "Quaternion.h"
#include <limits>
#include "PropStream.h"
#include <AB/ProtocolCodes.h>
#include "NetworkMessage.h"
#include "Transformation.h"
#include "BoundingBox.h"
#include "Octree.h"
#include "CollisionShape.h"
#include <AB/Entities/Character.h>
#include <AB/ProtocolCodes.h>
#include "IdGenerator.h"
#include "StateComp.h"
#include "Variant.h"

namespace Game {

enum ObjerctAttr : uint8_t
{
    ObjectAttrName = 1,

    // For serialization
    ObjectAttrEnd = 254
};

class Game;
class Actor;
class Npc;
class Player;

class GameObject : public std::enable_shared_from_this<GameObject>
{
    friend class Math::Octant;
    friend class Math::Octree;
private:
    static Utils::IdGenerator<uint32_t> objectIds_;
    std::unique_ptr<Math::CollisionShape> collisionShape_;
    std::vector<std::shared_ptr<GameObject>> _LuaQueryObjects(float radius);
    std::vector<std::shared_ptr<GameObject>> _LuaRaycast(float x, float y, float z);
    std::shared_ptr<Actor> _LuaAsActor();
    std::shared_ptr<Npc> _LuaAsNpc();
    std::shared_ptr<Player> _LuaAsPlayer();
    void _LuaSetPosition(float x, float y, float z);
    void _LuaSetRotation(float y);
    void _LuaSetScale(float x, float y, float z);
    std::vector<float> _LuaGetPosition() const;
    float _LuaGetRotation() const;
    std::vector<float> _LuaGetScale() const;
    void _LuaSetBoundingBox(float minX, float minY, float minZ,
        float maxX, float maxY, float maxZ);
    std::string _LuaGetVarString(const std::string& name);
    void _LuaSetVarString(const std::string& name, const std::string& value);
    float _LuaGetVarNumber(const std::string& name);
    void _LuaSetVarNumber(const std::string& name, float value);
protected:
    Utils::VariantMap variables_;
    std::weak_ptr<Game> game_;
    /// Octree octant.
    Math::Octant* octant_;
    float sortValue_;
    uint32_t GetNewId()
    {
        return objectIds_.Next();
    }
    void AddToOctree();
    void RemoveFromOctree();
public:
    static void RegisterLua(kaguya::State& state);

    GameObject();
    virtual ~GameObject();

    template <typename T>
    std::shared_ptr<T> GetThis()
    {
        return std::static_pointer_cast<T>(shared_from_this());
    }
    template <typename T>
    std::shared_ptr<T> GetThisDynamic()
    {
        return std::dynamic_pointer_cast<T>(shared_from_this());
    }

    virtual void Update(uint32_t timeElapsed, Net::NetworkMessage& message) {
        AB_UNUSED(message);
        AB_UNUSED(timeElapsed);
    }

    void SetCollisionShape(std::unique_ptr<Math::CollisionShape> shape)
    {
        collisionShape_ = std::move(shape);
    }
    uint32_t GetId() const { return id_; }
    std::shared_ptr<Game> GetGame() const
    {
        return game_.lock();
    }
    virtual void SetGame(std::shared_ptr<Game> game)
    {
        RemoveFromOctree();
        game_ = game;
        if (game)
            AddToOctree();
    }
    void SetCollisionMask(uint32_t mask)
    {
        collisionMask_ = mask;
    }
    uint32_t GetCollisionMask() const { return collisionMask_; }
    Math::CollisionShape* GetCollisionShape() const
    {
        if (!collisionShape_)
            return nullptr;
        return collisionShape_.get();
    }
    bool Collides(GameObject* other, Math::Vector3& move) const;

    virtual AB::GameProtocol::GameObjectType GetType() const
    {
        return AB::GameProtocol::ObjectTypeStatic;
    }

    const Utils::Variant& GetVar(const std::string& name) const;
    void SetVar(const std::string& name, const Utils::Variant& val);
    /// Process octree raycast. May be called from a worker thread.
    virtual void ProcessRayQuery(const Math::RayOctreeQuery& query, std::vector<Math::RayQueryResult>& results);
    void SetSortValue(float value) { sortValue_ = value; }
    float GetSortValue() const { return sortValue_; }

    /// Return octree octant.
    Math::Octant* GetOctant() const
    {
        return octant_;
    }
    /// Move into another octree octant.
    void SetOctant(Math::Octant* octant)
    {
        octant_ = octant;
    }

    bool Raycast(std::vector<GameObject*>& result, const Math::Vector3& direction);

    virtual bool Serialize(IO::PropWriteStream& stream);

    virtual std::string GetName() const { return name_; }

    Math::Transformation transformation_;
    /// Auto ID, not DB ID
    uint32_t id_;
    std::string name_;
    Components::StateComp stateComp_;
    /// Occluder flag. An object that can hide another object from view.
    bool occluder_;
    /// Occludee flag. An object that can be hidden from view (because it is occluded by another object) but that cannot, itself, hide another object from view.
    bool occludee_;
    uint32_t collisionMask_;
    virtual Math::BoundingBox GetWorldBoundingBox() const
    {
        if (!collisionShape_)
            return Math::BoundingBox();
        return collisionShape_->GetWorldBoundingBox(transformation_.GetMatrix());
    }
    virtual Math::BoundingBox GetBoundingBox() const
    {
        if (!collisionShape_)
            return Math::BoundingBox();
        return collisionShape_->GetBoundingBox();
    }
    bool QueryObjects(std::vector<GameObject*>& result, float radius);
    bool QueryObjects(std::vector<GameObject*>& result, const Math::BoundingBox& box);

    virtual void WriteSpawnData(Net::NetworkMessage&) { }

    virtual void OnSelected(std::shared_ptr<Actor>) { }
    virtual void OnClicked(std::shared_ptr<Actor>) { }
    virtual void OnCollide(std::shared_ptr<Actor> actor);
    virtual void OnTrigger(std::shared_ptr<Actor>) { }
};

inline bool CompareObjects(GameObject* lhs, GameObject* rhs)
{
    return lhs->GetSortValue() < rhs->GetSortValue();
}

}
