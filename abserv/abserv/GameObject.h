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

namespace Game {

enum ObjerctAttr : uint8_t
{
    ObjectAttrName = 1,

    // For serialization
    ObjectAttrEnd = 254
};

class Game;

class GameObject : public std::enable_shared_from_this<GameObject>
{
    friend class Math::Octant;
    friend class Math::Octree;
private:
    static uint32_t objectIds_;
protected:
    std::weak_ptr<Game> game_;
    /// Octree octant.
    Math::Octant* octant_;
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

    uint32_t GetId() const { return id_; }
    std::shared_ptr<Game> GetGame() const
    {
        return game_.lock();
    }
    void SetGame(std::shared_ptr<Game> game)
    {
        RemoveFromOctree();
        game_ = game;
        if (game)
            AddToOctree();
    }
    virtual AB::GameProtocol::GameObjectType GetType() const
    {
        return AB::GameProtocol::ObjectTypeUnknown;
    }
    virtual bool Serialize(IO::PropWriteStream& stream);

    /// Return octree octant.
    Math::Octant* GetOctant() const { return octant_; }
    /// Move into another octree octant.
    void SetOctant(Math::Octant* octant) { octant_ = octant; }

    virtual std::string GetName() const { return "Unknown"; }
    Math::Transformation transformation_;
    Math::BoundingBox boundingBox_;
    /// Auto ID, not DB ID
    uint32_t id_;
    /// Occluder flag.
    bool occluder_;
    /// Occludee flag.
    bool ocludee_;
    Math::BoundingBox GetWorldBoundingBox() const
    {
        return boundingBox_.Transformed(transformation_.GetMatrix());
    }

};

}
