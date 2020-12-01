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

#pragma once

#include <abshared/Damage.h>
#include <absmath/OctreeObject.h>
#include <absmath/OctreeQuery.h>
#include "StateComp.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Skill.h>
#include <AB/ProtocolCodes.h>
#include <eastl.hpp>
#include <sa/PropStream.h>
#include <abscommon/Variant.h>
#include <abshared/Mechanic.h>
#include <absmath/BoundingBox.h>
#include <absmath/CollisionShape.h>
#include <absmath/Matrix4.h>
#include <absmath/Transformation.h>
#include <kaguya/kaguya.hpp>
#include <mutex>
#include <sa/Events.h>
#include <sa/IdGenerator.h>
#include <sa/Iteration.h>
#include <sa/Noncopyable.h>
#include <sa/StringHash.h>

namespace Net {
class NetworkMessage;
}

namespace Game {

namespace Components {
class TriggerComp;
class CollisionComp;
}

enum ObjerctAttr : uint8_t
{
    ObjectAttrName = 1,

    // For serialization
    ObjectAttrEnd = 254
};

class Game;
class Actor;
class Model;
class Npc;
class Player;
class AreaOfEffect;
class Skill;

inline constexpr sa::event_t EVENT_ON_CLICKED = sa::StringHash("OnClicked");
inline constexpr sa::event_t EVENT_ON_COLLIDE = sa::StringHash("OnCollide");
inline constexpr sa::event_t EVENT_ON_LEFTAREA = sa::StringHash("OnLeftArea");
inline constexpr sa::event_t EVENT_ON_SELECTED = sa::StringHash("OnSelected");
inline constexpr sa::event_t EVENT_ON_TRIGGER = sa::StringHash("OnTrigger");
inline constexpr sa::event_t EVENT_ON_STATECHANGE = sa::StringHash("OnStateChange");
inline constexpr sa::event_t EVENT_ON_INTERACT = sa::StringHash("OnInteract");
inline constexpr sa::event_t EVENT_ON_CANCELALL = sa::StringHash("OnCancelAll");

using GameObjectEvents = sa::Events<
    void(void),
    void(uint32_t),
    void(int),
    void(bool&),
    void(AB::GameProtocol::CreatureState, AB::GameProtocol::CreatureState),                   // OnStateChange(old,new)
    void(int, int),
    void(GameObject*),
    void(Actor*),
    void(Skill*),
    void(Actor*, Actor*),
    void(Actor*, uint32_t, bool&),
    void(Actor*, DamageType, int32_t, bool&),
    void(AB::Entities::SkillType, Skill*, bool&),
    void(uint32_t, AB::GameProtocol::ObjectCallType, int),   // OnPingObject
    void(Actor*, Skill*, bool&),                             // OnUseSkill, OnSkillTargeted
    void(Actor*, bool&),
    void(Actor*, int&),
    void(AB::GameProtocol::CommandType, const std::string&, Net::NetworkMessage&)
>;

inline const float AVERAGE_BB_EXTENDS = 0.3f;

class GameObject : public Math::OctreeObject, public ea::enable_shared_from_this<GameObject>
{
    NON_COPYABLE(GameObject)
public:
    static sa::IdGenerator<uint32_t> objectIds_;
private:
    int64_t removeAt_{ 0 };
    bool hasGame_{ false };
    ea::unique_ptr<Math::AbstractCollisionShape> collisionShape_;
    ea::shared_ptr<Model> model_;
    std::vector<GameObject*> _LuaQueryObjects(float radius);
    std::vector<GameObject*> _LuaRaycast(const Math::StdVector3& direction);
    /// Raycast to destination point
    std::vector<GameObject*> _LuaRaycastTo(const Math::StdVector3& destination);
    Actor* _LuaAsActor();
    Npc* _LuaAsNpc();
    Player* _LuaAsPlayer();
    AreaOfEffect* _LuaAsAOE();
    void _LuaSetPosition(const Math::StdVector3& pos);
    void _LuaSetRotation(float y);
    void _LuaSetScale(const Math::StdVector3& scale);
    void _LuaSetScaleSimple(float value);
    Math::StdVector3 _LuaGetPosition() const;
    float _LuaGetRotation() const;
    Math::StdVector3 _LuaGetScale() const;
    void _LuaSetBoundingBox(const Math::StdVector3& min, const Math::StdVector3& max);
    void _LuaSetBoundingSize(const Math::StdVector3& size);
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
    std::vector<Actor*> _LuaGetActorsInRange(Ranges range) const;
    uint32_t _LuaGetCollisionMask() const { return GetCollisionMask(); }
    void _LuaSetCollisionMask(uint32_t value) { return SetCollisionMask(value); }
    uint32_t _LuaGetCollisionLayer() const { return GetCollisionLayer(); }
    void _LuaSetCollisionLayer(uint32_t value) { return SetCollisionLayer(value); }
protected:
    std::string name_;
    Utils::VariantMap variables_;
    ea::weak_ptr<Game> game_;
    GameObjectEvents events_;
    ea::map<Ranges, ea::set<uint32_t>> ranges_;
    void UpdateRanges();
    uint32_t GetNewId()
    {
        return objectIds_.Next();
    }
    void AddToOctree();
    void RemoveFromOctree();
public:
    static void RegisterLua(kaguya::State& state);
    /// Let's make the head 1.7m above the ground
    static constexpr Math::Vector3 HeadOffset{ 0.0f, 1.7f, 0.0f };
    /// Center of body
    static constexpr Math::Vector3 BodyOffset{ 0.0f, 0.8f, 0.0f };

    GameObject();
    virtual ~GameObject();

    // Return smart pointer
    template <typename T>
    inline ea::shared_ptr<T> GetPtr();

    virtual void Update(uint32_t timeElapsed, Net::NetworkMessage& message);

    void SetBoundingSize(const Math::Vector3& size);
    void SetCollisionShape(ea::unique_ptr<Math::AbstractCollisionShape> shape)
    {
        collisionShape_ = std::move(shape);
    }
    void SetModel(ea::shared_ptr<Model> model);
    uint32_t GetId() const { return id_; }
    ea::shared_ptr<Game> GetGame() const
    {
        return game_.lock();
    }
    // Returns true when the object is currently in an outpost
    bool IsInOutpost() const;
    virtual void SetGame(ea::shared_ptr<Game> game)
    {
        RemoveFromOctree();
        game_ = game;
        if (game)
            AddToOctree();
        hasGame_ = !!game;
    }
    bool HasGame() const { return hasGame_; }
    bool IsSelectable() const { return selectable_; }
    void SetSelectable(bool value) { selectable_ = value; }
    Math::AbstractCollisionShape* GetCollisionShape() const
    {
        if (!collisionShape_)
            return nullptr;
        return collisionShape_.get();
    }
    bool Collides(const Math::OctreeObject* other, const Math::Vector3& velocity, Math::Vector3& move) const;
    void Collides(Math::OctreeObject** others, size_t count, const Math::Vector3& velocity,
        const std::function<Iteration(GameObject& other, const Math::Vector3& move, bool& updateTrans)>& callback) const;
    /// Get the distance to another object
    float GetDistance(const GameObject* other) const
    {
        if (!other)
            return Math::M_INFINITE;
        return transformation_.position_.Distance(other->transformation_.position_);
    }

    /// Test if object is in our range
    bool IsInRange(Ranges range, const GameObject* object) const;
    /// Check if the object is within maxDist
    bool IsCloserThan(float maxDist, const GameObject* object) const;
    /// Allows to execute a functor/lambda on the objects in range
    void VisitInRange(Ranges range, const std::function<Iteration(GameObject&)>& func) const;
    /// Returns the closest actor or nullptr
    /// @param[in] undestroyable If true include undestroyable actors
    /// @param[in] unselectable If true includes unselectable actors
    Actor* GetClosestActor(bool undestroyable, bool unselectable);
    Actor* GetClosestActor(const std::function<bool(const Actor& actor)>& callback);

    virtual AB::GameProtocol::GameObjectType GetType() const
    {
        return AB::GameProtocol::GameObjectType::Static;
    }
    bool IsActorType() const { return GetType() >= AB::GameProtocol::GameObjectType::Projectile; }
    bool IsPlayerOrNpcType() const
    {
        const auto t = GetType();
        return t == AB::GameProtocol::GameObjectType::Npc || t == AB::GameProtocol::GameObjectType::Player;
    }
    const Math::Vector3& GetPosition() const
    {
        return transformation_.position_;
    }

    const Utils::Variant& GetVar(const std::string& name) const;
    void SetVar(const std::string& name, const Utils::Variant& val);
    /// Process octree raycast.
    void ProcessRayQuery(const Math::RayOctreeQuery& query, ea::vector<Math::RayQueryResult>& results) override;

    bool Raycast(ea::vector<GameObject*>& result, const Math::Vector3& direction, float maxDist = Math::M_INFINITE) const;
    bool Raycast(ea::vector<GameObject*>& result, const Math::Vector3& position, const Math::Vector3& direction, float maxDist = Math::M_INFINITE) const;
    bool RaycastWithResult(ea::vector<Math::RayQueryResult>& result, const Math::Vector3& position, const Math::Vector3& direction, float maxDist = Math::M_INFINITE) const;
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
    auto CallEventOne(sa::event_t id, _CArgs&& ... _Args)
    {
        return events_.CallOne<Signature, _CArgs...>(id, std::forward<_CArgs>(_Args)...);
    }
    /// Calls all subscribers
    template <typename Signature, typename... _CArgs>
    auto CallEvent(sa::event_t id, _CArgs&& ... _Args)
    {
        return events_.CallAll<Signature, _CArgs...>(id, std::forward<_CArgs>(_Args)...);
    }

    virtual bool Serialize(sa::PropWriteStream& stream);

    virtual const std::string& GetName() const { return name_; }
    virtual void SetName(const std::string& name) { name_ = name; }
    virtual void SetState(AB::GameProtocol::CreatureState state);

    Math::Transformation transformation_;
    /// Auto ID, not DB ID
    uint32_t id_;
    /// Can not be selected by a player. Majority of objects is not selectable by the player so lets make it true by default.
    bool selectable_{ false };
    Components::StateComp stateComp_;
    ea::unique_ptr<Components::TriggerComp> triggerComp_;
    Math::BoundingBox GetWorldBoundingBox() const override
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
    bool QueryObjects(ea::vector<Math::OctreeObject*>& result, float radius, const Math::OctreeMatcher* matcher = nullptr);
    bool QueryObjects(ea::vector<Math::OctreeObject*>& result, const Math::BoundingBox& box, const Math::OctreeMatcher* matcher = nullptr);

    uint32_t GetRetriggerTimout() const;
    void SetRetriggerTimout(uint32_t value);
    bool IsTrigger() const;
    void SetTrigger(bool value);

    virtual void WriteSpawnData(Net::NetworkMessage& msg);

    friend std::ostream& operator << (std::ostream& os, const GameObject& value)
    {
        return os << "GameObject{" << static_cast<int>(value.GetType()) << "} id " << value.id_ << ": " << value.GetName();
    }
};

template <typename T>
inline bool Is(const GameObject&)
{
    return false;
}

/// Returns false when obj is null
template <typename T>
inline bool Is(const GameObject* obj)
{
    return obj && Is<T>(*obj);
}

template <>
inline bool Is<GameObject>(const GameObject&)
{
    return true;
}

template <typename T>
inline const T& To(const GameObject& obj)
{
    ASSERT(Is<T>(obj));
    return static_cast<const T&>(obj);
}

template <typename T>
inline T& To(GameObject& obj)
{
    ASSERT(Is<T>(obj));
    return static_cast<T&>(obj);
}

template <typename T>
inline const T* To(const GameObject* obj)
{
    if (!Is<T>(obj))
        return nullptr;
    return static_cast<const T*>(obj);
}

template <typename T>
inline T* To(GameObject* obj)
{
    if (!Is<T>(obj))
        return nullptr;
    return static_cast<T*>(obj);
}

template <typename T>
inline ea::shared_ptr<T> GameObject::GetPtr()
{
    if (Is<T>(*this))
        return ea::static_pointer_cast<T>(shared_from_this());
    return ea::shared_ptr<T>();
}

template<typename T>
class ObjectTypeMatcher final : public Math::OctreeMatcher
{
public:
    bool Matches(const GameObject* object) const override
    {
        return Is<T>(object);
    }
};

class SentToPlayerMatcher final : public Math::OctreeMatcher
{
public:
    bool Matches(const Math::OctreeObject* object) const override
    {
        return object && static_cast<const GameObject*>(object)->GetType() > AB::GameProtocol::GameObjectType::__SentToPlayer;
    }
};

}
