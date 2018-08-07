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

namespace Game {

enum ObjerctAttr : uint8_t
{
    ObjectAttrName = 1,

    // For serialization
    ObjectAttrEnd = 254
};

class Game;
class Creature;

class GameObject : public std::enable_shared_from_this<GameObject>
{
    friend class Math::Octant;
    friend class Math::Octree;
private:
    static uint32_t objectIds_;
    std::unique_ptr<Math::CollisionShape> collisionShape_;
protected:
    std::weak_ptr<Game> game_;
    /// Octree octant.
    Math::Octant* octant_;
    float sortValue_;
    uint32_t GetNewId()
    {
        if (objectIds_ >= std::numeric_limits<uint32_t>::max())
            objectIds_ = 0;
        return ++objectIds_;
    }
    template <typename T>
    std::shared_ptr<T> GetThis()
    {
        return std::static_pointer_cast<T>(shared_from_this());
    }
    void AddToOctree();
    void RemoveFromOctree();
public:
    static void RegisterLua(kaguya::State& state);

    GameObject();
    virtual ~GameObject();

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
        return AB::GameProtocol::ObjectTypeUnknown;
    }
    /// Process octree raycast. May be called from a worker thread.
    virtual void ProcessRayQuery(const Math::RayOctreeQuery& query, std::vector<Math::RayQueryResult>& results);
    void SetSortValue(float value) { sortValue_ = value; }
    float GetSortValue() const { return sortValue_; }

    virtual bool Serialize(IO::PropWriteStream& stream);

    /// Return octree octant.
    Math::Octant* GetOctant() const { return octant_; }
    /// Move into another octree octant.
    void SetOctant(Math::Octant* octant) { octant_ = octant; }

    virtual std::string GetName() const { return name_; }
    virtual AB::Entities::CharacterSex GetSex() const
    {
        return AB::Entities::CharacterSexUnknown;
    }
    virtual uint32_t GetModelIndex() const
    {
        return 0;
    }
    virtual uint32_t GetProfIndex() const
    {
        return 0;
    }
    virtual uint32_t GetProf2Index() const
    {
        return 0;
    }
    Math::Transformation transformation_;
    /// Auto ID, not DB ID
    uint32_t id_;
    std::string name_;
    AB::GameProtocol::CreatureState creatureState_;
    /// Occluder flag. An object that can hide another object from view.
    bool occluder_;
    /// Occludee flag. An object that can be hidden from view (because it is occluded by another object) but that cannot, itself, hide another object from view.
    bool occludee_;
    uint32_t collisionMask_;
    Math::BoundingBox GetWorldBoundingBox() const
    {
        if (!collisionShape_)
            return Math::BoundingBox();
        return collisionShape_->GetWorldBoundingBox(transformation_.GetMatrix());
    }
    bool QueryObjects(std::vector<GameObject*>& result, float radius);
    bool QueryObjects(std::vector<GameObject*>& result, const Math::BoundingBox& box);

    virtual void OnSelected(std::shared_ptr<Creature>) { }
    virtual void OnCollide(std::shared_ptr<Creature>) { }
};

inline bool CompareObjects(GameObject* lhs, GameObject* rhs)
{
    return lhs->GetSortValue() < rhs->GetSortValue();
}

}
