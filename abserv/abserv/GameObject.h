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
#include "TriggerComp.h"
#include "Variant.h"
#include <unordered_set>
#include "Matrix4.h"
#include "Mechanic.h"
#include "CollisionComp.h"
#include "MoveComp.h"
#include <kaguya/kaguya.hpp>

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
    friend class Components::MoveComp;
public:
    static Utils::IdGenerator<uint32_t> objectIds_;
private:
    std::unique_ptr<Math::CollisionShape> collisionShape_;
    std::vector<GameObject*> _LuaQueryObjects(float radius);
    std::vector<GameObject*> _LuaRaycast(float x, float y, float z);
    Actor* _LuaAsActor();
    Npc* _LuaAsNpc();
    Player* _LuaAsPlayer();
    void _LuaSetPosition(float x, float y, float z);
    void _LuaSetRotation(float y);
    void _LuaSetScale(float x, float y, float z);
    void _LuaSetScaleSimple(float value);
    std::vector<float> _LuaGetPosition() const;
    float _LuaGetRotation() const;
    std::vector<float> _LuaGetScale() const;
    void _LuaSetBoundingBox(float minX, float minY, float minZ,
        float maxX, float maxY, float maxZ);
    void _LuaSetBoundingSize(float x, float y, float z);
    std::string _LuaGetVarString(const std::string& name);
    void _LuaSetVarString(const std::string& name, const std::string& value);
    float _LuaGetVarNumber(const std::string& name);
    void _LuaSetVarNumber(const std::string& name, float value);
    Game* _LuaGetGame();
    /// Call a method on of the game script. data is optional
    void _LuaCallGameEvent(const std::string& name, GameObject* data);
protected:
    std::mutex lock_;
    Utils::VariantMap variables_;
    std::weak_ptr<Game> game_;
    /// Octree octant.
    Math::Octant* octant_;
    float sortValue_;
    std::map<Ranges, std::vector<std::weak_ptr<GameObject>>> ranges_;
    void UpdateRanges();
    uint32_t GetNewId()
    {
        return objectIds_.Next();
    }
    void AddToOctree();
    void RemoveFromOctree();
public:
    static void RegisterLua(kaguya::State& state);
    /// The head is not on the ground.
    static const Math::Vector3 HeadOffset;
    static const Math::Vector3 BodyOffset;

    GameObject();
    // non-copyable
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;
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

    virtual void Update(uint32_t timeElapsed, Net::NetworkMessage& message);

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
    bool Collides(GameObject* other, const Math::Vector3& velocity, Math::Vector3& move) const;
    /// Get the distance to another object
    float GetDistance(GameObject* other) const
    {
        if (!other)
            return std::numeric_limits<float>::max();
        return transformation_.position_.Distance(other->transformation_.position_);
    }

    /// Test if object is in our range
    bool IsInRange(Ranges range, GameObject* object)
    {
        if (!object)
            return false;
        if (range == Ranges::Map)
            return true;
        // Don't calculate the distance now, but use previously calculated values.
        const auto& r = ranges_[range];
        const auto it = std::find_if(r.begin(), r.end(), [object](const std::weak_ptr<GameObject>& current) -> bool
        {
            if (auto c = current.lock())
                return c->id_ == object->id_;
            return false;
        });
        return it != r.end();
    }
    /// Allows to execute a functor/lambda on the objects in range
    template<typename Func>
    void VisitInRange(Ranges range, Func&& func)
    {
        // May be called from the AI thread so lock it
        std::lock_guard<std::mutex> lock(lock_);
        for (const auto o : ranges_[range])
        {
            if (auto so = o.lock())
                func(so);
        }
    }
    std::vector<Actor*> GetActorsInRange(Ranges range);

    virtual AB::GameProtocol::GameObjectType GetType() const
    {
        return AB::GameProtocol::ObjectTypeStatic;
    }

    const Math::Vector3& GetPosition() const
    {
        return transformation_.position_;
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
    std::unique_ptr<Components::TriggerComp> triggerComp_;
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

    uint32_t GetRetriggerTimout() const
    {
        if (!triggerComp_)
            return 0;
        return triggerComp_->retriggerTimeout_;
    }
    void SetRetriggerTimout(uint32_t value)
    {
        if (!triggerComp_)
            triggerComp_ = std::make_unique<Components::TriggerComp>(*this);
        triggerComp_->retriggerTimeout_ = value;
    }
    bool IsTrigger() const
    {
        return triggerComp_ && triggerComp_->trigger_;
    }
    void SetTrigger(bool value)
    {
        if (!triggerComp_)
            triggerComp_ = std::make_unique<Components::TriggerComp>(*this);
        triggerComp_->trigger_ = value;
    }
    virtual void WriteSpawnData(Net::NetworkMessage&) { }

    virtual void OnSelected(Actor*) { }
    virtual void OnClicked(Actor*) { }
    virtual void OnCollide(GameObject* other);
    /// Object entered the area
    virtual void OnTrigger(GameObject*) { }
    /// Object left the area. Opposite to OnTrigger
    virtual void OnLeftArea(GameObject*) { }
};

inline bool CompareObjects(GameObject* lhs, GameObject* rhs)
{
    return lhs->GetSortValue() < rhs->GetSortValue();
}

}
