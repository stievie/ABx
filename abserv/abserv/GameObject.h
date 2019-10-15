#pragma once

#include <mutex>
#include "Vector3.h"
#include "Quaternion.h"
#include "PropStream.h"
#include <AB/ProtocolCodes.h>
#include "NetworkMessage.h"
#include "Transformation.h"
#include "BoundingBox.h"
#include "Octree.h"
#include "CollisionShape.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Skill.h>
#include <AB/ProtocolCodes.h>
#include <sa/IdGenerator.h>
#include "StateComp.h"
#include "TriggerComp.h"
#include "Variant.h"
#include "Matrix4.h"
#include "CollisionComp.h"
#include "MoveComp.h"
#include "InputComp.h"
#include <sa/Iteration.h>
#include "Mechanic.h"
#include <kaguya/kaguya.hpp>
#include <sa/Events.h>
#include "Damage.h"
#include "StringHash.h"

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
class Skill;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4307)
#endif
static constexpr sa::event_t EVENT_ON_CLICKED = Utils::StringHash("OnClicked");
static constexpr sa::event_t EVENT_ON_COLLIDE = Utils::StringHash("OnCollide");
static constexpr sa::event_t EVENT_ON_LEFTAREA = Utils::StringHash("OnLeftArea");
static constexpr sa::event_t EVENT_ON_SELECTED = Utils::StringHash("OnSelected");
static constexpr sa::event_t EVENT_ON_TRIGGER = Utils::StringHash("OnTrigger");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

using GameObjectEvents = sa::Events<
    void(void),
    void(uint32_t),
    void(int),
    void(bool&),
    void(int, int),
    void(GameObject*),
    void(Actor*),
    void(Skill*),
    void(Actor*, uint32_t, bool&),
    void(Actor*, DamageType, int32_t, bool&),
    void(AB::Entities::SkillType, Skill*, bool&),
    void(uint32_t, AB::GameProtocol::ObjectCallType, int),   // OnPingObject
    void(Actor*, Skill*, bool&),                                    // OnUseSkill, OnSkillTargeted
    void(Actor*, bool&),
    void(Actor*, int&),
    void(AB::GameProtocol::CommandTypes, const std::string&, Net::NetworkMessage&)
>;

class GameObject : public std::enable_shared_from_this<GameObject>
{
    friend class Math::Octant;
    friend class Math::Octree;
public:
    static sa::IdGenerator<uint32_t> objectIds_;
private:
    int64_t removeAt_{ 0 };
    std::unique_ptr<Math::CollisionShape> collisionShape_{ nullptr };
    std::vector<GameObject*> _LuaQueryObjects(float radius);
    std::vector<GameObject*> _LuaRaycast(const Math::STLVector3& direction);
    /// Raycast to destination point
    std::vector<GameObject*> _LuaRaycastTo(const Math::STLVector3& destination);
    Actor* _LuaAsActor();
    Npc* _LuaAsNpc();
    Player* _LuaAsPlayer();
    void _LuaSetPosition(const Math::STLVector3& pos);
    void _LuaSetRotation(float y);
    void _LuaSetScale(const Math::STLVector3& scale);
    void _LuaSetScaleSimple(float value);
    Math::STLVector3 _LuaGetPosition() const;
    float _LuaGetRotation() const;
    Math::STLVector3 _LuaGetScale() const;
    void _LuaSetBoundingBox(const Math::STLVector3& min, const Math::STLVector3& max);
    void _LuaSetBoundingSize(const Math::STLVector3& size);
    std::string _LuaGetVarString(const std::string& name);
    void _LuaSetVarString(const std::string& name, const std::string& value);
    float _LuaGetVarNumber(const std::string& name);
    void _LuaSetVarNumber(const std::string& name, float value);
    Game* _LuaGetGame();
    int _LuaGetState();
    void _LuaSetState(int state);
    /// Call a method of the game script. data is optional
    void _LuaCallGameEvent(const std::string& name, GameObject* data);
    Actor* _LuaGetClosestActor(bool undestroyable, bool unselectable);
    /// Returns all object inside the collision shape. This object must be a Trigger, i.e. it returns only objects when SetTrigger(true) was called before.
    std::vector<GameObject*> _LuaGetObjectsInside();
    bool _LuaIsObjectInSight(const GameObject* object) const;
protected:
    std::mutex lock_;
    std::string name_;
    Utils::VariantMap variables_;
    std::weak_ptr<Game> game_;
    GameObjectEvents events_;
    /// Octree octant.
    Math::Octant* octant_{ nullptr };
    float sortValue_{ 0.0f };
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

    void SetBoundingSize(const Math::Vector3& size);
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
    bool IsSelectable() const { return selectable_; }
    void SetSelectable(bool value) { selectable_ = value; }
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
    bool Collides(const GameObject* other, const Math::Vector3& velocity, Math::Vector3& move) const;
    /// Get the distance to another object
    float GetDistance(const GameObject* other) const
    {
        if (!other)
            return Math::M_INFINITE;
        return transformation_.position_.Distance(other->transformation_.position_);
    }

    /// Test if object is in our range
    bool IsInRange(Ranges range, const GameObject* object) const
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
        const auto it = std::find_if(r.begin(), r.end(), [object](const std::weak_ptr<GameObject>& current) -> bool
        {
            if (auto c = current.lock())
                return c->id_ == object->id_;
            return false;
        });
        return it != r.end();
    }
    /// Check if the object is within maxDist
    bool IsCloserThan(float maxDist, const GameObject* object) const;
    /// Allows to execute a functor/lambda on the objects in range
    template<typename Func>
    void VisitInRange(Ranges range, const Func& func)
    {
        // May be called from the AI thread so lock it
        std::lock_guard<std::mutex> lock(lock_);
        for (const auto& o : ranges_[range])
        {
            if (auto so = o.lock())
                if (func(*so) != Iteration::Continue)
                    break;
        }
    }
    std::vector<Actor*> GetActorsInRange(Ranges range);
    /// Returns the closest actor or nullptr
    /// @param[in] undestroyable If true include undestroyable actors
    /// @param[in] unselectable If true includes unselectable actors
    Actor* GetClosestActor(bool undestroyable, bool unselectable);
    Actor* GetClosestActor(const std::function<bool(const Actor& actor)>& callback);

    virtual AB::GameProtocol::GameObjectType GetType() const
    {
        return AB::GameProtocol::ObjectTypeStatic;
    }
    bool IsActorType() const { return GetType() >= AB::GameProtocol::ObjectTypeProjectile; }
    bool IsPlayerOrNpcType() const
    {
        const auto t = GetType();
        return t == AB::GameProtocol::ObjectTypeNpc || t == AB::GameProtocol::ObjectTypePlayer;
    }
    const Math::Vector3& GetPosition() const
    {
        return transformation_.position_;
    }

    const Utils::Variant& GetVar(const std::string& name) const;
    void SetVar(const std::string& name, const Utils::Variant& val);
    /// Process octree raycast.
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

    bool Raycast(std::vector<GameObject*>& result, const Math::Vector3& direction, float maxDist = Math::M_INFINITE) const;
    bool Raycast(std::vector<GameObject*>& result, const Math::Vector3& position, const Math::Vector3& direction, float maxDist = Math::M_INFINITE) const;
    bool IsObjectInSight(const GameObject& object) const;
    /// Remove this object from scene
    void Remove();
    /// Remove this object in time ms
    void RemoveIn(uint32_t time);

    template <typename Signature>
    size_t SubscribeEvent(sa::event_t id, std::function<Signature>&& func)
    {
        return events_.Subscribe<Signature>(id, std::move(func));
    }
    template <typename Signature>
    void UnsubscribeEvent(sa::event_t id, size_t index)
    {
        events_.Unsubscribe<Signature>(id, index);
    }
    /// Call the first subscriber
    template <typename Signature, typename... _CArgs>
    auto CallEventOne(sa::event_t id, _CArgs&& ... _Args) -> typename std::invoke_result<Signature, _CArgs...>::type
    {
        return events_.CallOne<Signature, _CArgs...>(id, std::forward<_CArgs>(_Args)...);
    }
    /// Calls all subscribers
    template <typename Signature, typename... _CArgs>
    auto CallEvent(sa::event_t id, _CArgs&& ... _Args)
    {
        return events_.CallAll<Signature, _CArgs...>(id, std::forward<_CArgs>(_Args)...);
    }

    virtual bool Serialize(IO::PropWriteStream& stream);

    virtual const std::string& GetName() const { return name_; }
    virtual void SetName(const std::string& name) { name_ = name; }
    virtual void SetState(AB::GameProtocol::CreatureState state);

    Math::Transformation transformation_;
    /// Auto ID, not DB ID
    uint32_t id_;
    /// Can not be selected by a player. Majority of objects is not selectable by the player so lets make it true by default.
    bool selectable_{ false };
    Components::StateComp stateComp_;
    std::unique_ptr<Components::TriggerComp> triggerComp_;
    /// Occluder flag. An object that can hide another object from view.
    bool occluder_{ false };
    /// Occludee flag. An object that can be hidden from view (because it is occluded by another object) but that cannot, itself, hide another object from view.
    bool occludee_{ true };
    uint32_t collisionMask_{ 0xFFFFFFFF };     // Collides with all by default
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

    virtual void WriteSpawnData(Net::NetworkMessage& msg);
};

inline bool CompareObjects(GameObject* lhs, GameObject* rhs)
{
    return lhs->GetSortValue() < rhs->GetSortValue();
}

}
